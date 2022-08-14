#include "Texture.h"

#include "Logging/Logger.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "AssetSystem/ImageAsset.h"
#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Image.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Fence.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"
#include "RenderInterface/GenericRenderInterface/Queue.h"
#include "RenderInterface/GenericRenderInterface/Sampler.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"

namespace Hermes
{
	static RenderInterface::DataFormat ChooseFormatFromImageType(ImageFormat Format, size_t BytesPerChannel, bool IsSRGB)
	{
		using FormatCombinationHashType = size_t;
		constexpr auto ComputeHashForFormat = [](ImageFormat Format, uint8 BytesPerChannel,
		                               bool IsSRGB) -> FormatCombinationHashType
		{
			return (static_cast<uint8>(Format) << 16) | (BytesPerChannel << 8) | static_cast<uint8>(IsSRGB);
		};

		static const std::unordered_map<FormatCombinationHashType, RenderInterface::DataFormat> FormatCombinations =
		{
			{ ComputeHashForFormat(ImageFormat::R, 1, true), RenderInterface::DataFormat::R8SRGB },
			{ ComputeHashForFormat(ImageFormat::R, 1, false), RenderInterface::DataFormat::R8UnsignedNormalized },
			{ ComputeHashForFormat(ImageFormat::R, 2, false), RenderInterface::DataFormat::R16UnsignedNormalized },

			{ ComputeHashForFormat(ImageFormat::RG, 1, true), RenderInterface::DataFormat::R8G8SRGB },
			{ ComputeHashForFormat(ImageFormat::RG, 1, false), RenderInterface::DataFormat::R8G8UnsignedNormalized },
			{ ComputeHashForFormat(ImageFormat::RG, 2, false), RenderInterface::DataFormat::R16G16UnsignedNormalized },

			{ ComputeHashForFormat(ImageFormat::RA, 1, true), RenderInterface::DataFormat::R8G8SRGB },
			{ ComputeHashForFormat(ImageFormat::RA, 1, false), RenderInterface::DataFormat::R8G8UnsignedInteger },
			{ ComputeHashForFormat(ImageFormat::RA, 2, false), RenderInterface::DataFormat::R16G16UnsignedNormalized },

			{ ComputeHashForFormat(ImageFormat::RGBA, 1, true), RenderInterface::DataFormat::R8G8B8A8SRGB },
			{ ComputeHashForFormat(ImageFormat::RGBA, 1, false), RenderInterface::DataFormat::R8G8B8A8UnsignedNormalized },
			{ ComputeHashForFormat(ImageFormat::RGBA, 2, false), RenderInterface::DataFormat::R16G16B16A16UnsignedNormalized },

			{ ComputeHashForFormat(ImageFormat::RGBX, 1, true), RenderInterface::DataFormat::R8G8B8A8SRGB },
			{ ComputeHashForFormat(ImageFormat::RGBX, 1, false), RenderInterface::DataFormat::R8G8B8A8UnsignedNormalized },
			{ ComputeHashForFormat(ImageFormat::RGBX, 2, false), RenderInterface::DataFormat::R16G16B16A16UnsignedNormalized },

			{ ComputeHashForFormat(ImageFormat::HDR, 4, false), RenderInterface::DataFormat::R32G32B32SignedFloat }
		};

		auto It = FormatCombinations.find(ComputeHashForFormat(Format, static_cast<uint8>(BytesPerChannel), IsSRGB));
		if (It == FormatCombinations.end())
		{
			HERMES_LOG_ERROR(L"Cannot choose data format for the folowing image properties: image format %hhu; %llu bytes per channel; %s color space",
			                 static_cast<uint8>(Format), BytesPerChannel, (IsSRGB ? L"sRGB" : L"linear"));
			return RenderInterface::DataFormat::Undefined;
		}
		return It->second;
	}

	std::shared_ptr<Texture> Texture::CreateFromAsset(const ImageAsset& Source, bool UseAsSRGB, bool EnableMipMaps)
	{
		return std::shared_ptr<Texture>(new Texture(Source, UseAsSRGB, EnableMipMaps));
	}

	const RenderInterface::Image& Texture::GetRawImage() const
	{
		HERMES_ASSERT(IsReady());
		return *Image;
	}

	const RenderInterface::ImageView& Texture::GetDefaultView() const
	{
		return *DefaultView;
	}

	Vec2ui Texture::GetDimensions() const
	{
		return Dimensions;
	}

	uint32 Texture::GetMipLevelsCount() const
	{
		return Image->GetMipLevelsCount();
	}

	RenderInterface::DataFormat Texture::GetDataFormat() const
	{
		if (!Image)
			return RenderInterface::DataFormat::Undefined;
		return Image->GetDataFormat();
	}

	bool Texture::IsReady() const
	{
		return DataUploadFinished && Image != nullptr && Dimensions.LengthSq() > 0;
	}

	Texture::Texture(const ImageAsset& Source, bool UseAsSRGB, bool EnableMipMaps)
		: Dimensions(Source.GetDimensions())
	{
		uint32 BiggestDimension = Math::Max(Dimensions.X, Dimensions.Y);
		HERMES_ASSERT(BiggestDimension > 0);

		uint32 MipLevelCount;
		if (EnableMipMaps)
		{
			if (Source.HasPrecomputedMips())
			{
				MipLevelCount = Source.GetMipLevelCount();
			}
			else
			{
				MipLevelCount = Math::FloorLog2(BiggestDimension) + 1;
			}
		}
		else
		{
			MipLevelCount = 1;
		}

		auto Format = ChooseFormatFromImageType(Source.GetImageFormat(), Source.GetBytesPerChannel(), UseAsSRGB);
		Image = Renderer::Get().GetActiveDevice().CreateImage(Dimensions,
		                                                      RenderInterface::ImageUsageType::Sampled |
		                                                      RenderInterface::ImageUsageType::CopyDestination |
		                                                      RenderInterface::ImageUsageType::CopySource, Format,
		                                                      MipLevelCount, RenderInterface::ImageLayout::Undefined);

		// NOTE : normally, after loading image we will move it into
		// transfer source layout for further blitting to generate mip
		// maps and transition to shader read only layout will be done
		// in GPUInteractionUtilities::GenerateMipMaps(). However, if
		// user does not want to generate mip maps or if they are
		// precomputed(so we only upload them) then we have to perform
		// transition to shader read only layout immediately ourselves
		RenderInterface::ImageLayout LayoutAfterLoad;
		if (EnableMipMaps && !Source.HasPrecomputedMips())
		{
			LayoutAfterLoad = RenderInterface::ImageLayout::TransferSourceOptimal;
		}
		else
		{
			LayoutAfterLoad = RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
		}

		GPUInteractionUtilities::UploadDataToGPUImage(Source.GetRawData(), { 0, 0 }, Dimensions,
		                                              Source.GetBytesPerPixel(), 0, *Image,
		                                              RenderInterface::ImageLayout::Undefined, LayoutAfterLoad);

		if (EnableMipMaps)
		{
			if (Source.HasPrecomputedMips())
			{
				for (uint8 MipLevel = 1; MipLevel < Source.GetMipLevelCount(); MipLevel++)
				{
					GPUInteractionUtilities::UploadDataToGPUImage(Source.GetRawData(MipLevel), { 0, 0 },
					                                              Source.GetDimensions(MipLevel),
					                                              Source.GetBytesPerPixel(), MipLevel, *Image,
					                                              RenderInterface::ImageLayout::Undefined,
					                                              LayoutAfterLoad);
				}
			}
			else
			{
				GPUInteractionUtilities::GenerateMipMaps(*Image, RenderInterface::ImageLayout::TransferSourceOptimal,
				                                         RenderInterface::ImageLayout::ShaderReadOnlyOptimal);
			}
		}

		DataUploadFinished = true;

		DefaultView = Image->CreateDefaultImageView();
	}

	std::unique_ptr<CubemapTexture> CubemapTexture::CreateFromEquirectangularTexture(
		const Texture& EquirectangularTexture, RenderInterface::DataFormat PreferredFormat, bool EnableMipMaps)
	{
		return std::unique_ptr<CubemapTexture>(new CubemapTexture(EquirectangularTexture, PreferredFormat, EnableMipMaps));
	}

	CubemapTexture::CubemapTexture(const Texture& EquirectangularTexture, RenderInterface::DataFormat PreferredFormat,
	                               bool EnableMipMaps)
	{
		auto EquirectangularTextureDimensions = EquirectangularTexture.GetDimensions();

		// NOTE : cubemap is 4 times narrower and 2 times shorter
		auto CubemapDimensions = EquirectangularTextureDimensions / Vec2ui { 4, 2 };
		Dimensions = CubemapDimensions;
		
		auto CubemapFormat = PreferredFormat;

		uint32 MipMapCount = 1;
		if (EnableMipMaps)
			MipMapCount = Math::FloorLog2(Math::Max(CubemapDimensions.X, CubemapDimensions.Y)) + 1;

		auto& Device = Renderer::Get().GetActiveDevice();
		Image = Device.CreateCubemap(CubemapDimensions,
		                               RenderInterface::ImageUsageType::Sampled |
		                               RenderInterface::ImageUsageType::CopySource |
		                               RenderInterface::ImageUsageType::CopyDestination |
		                               RenderInterface::ImageUsageType::ColorAttachment, CubemapFormat, MipMapCount,
		                               RenderInterface::ImageLayout::Undefined);

		auto& Queue = Device.GetQueue(RenderInterface::QueueType::Render);
		auto CommandBuffer = Queue.CreateCommandBuffer(true);

		RenderInterface::RenderPassAttachment OutputAttachment = {};
		OutputAttachment.Type = RenderInterface::AttachmentType::Color;
		OutputAttachment.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		OutputAttachment.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;
		OutputAttachment.StoreOp = RenderInterface::AttachmentStoreOp::Store;
		OutputAttachment.StencilStoreOp = RenderInterface::AttachmentStoreOp::Store;
		OutputAttachment.Format = CubemapFormat;
		OutputAttachment.LayoutAtStart = RenderInterface::ImageLayout::ColorAttachmentOptimal;
		OutputAttachment.LayoutAtEnd = RenderInterface::ImageLayout::ColorAttachmentOptimal;
		auto RenderPass = Device.CreateRenderPass({ OutputAttachment });

		std::shared_ptr VertexShader = Device.CreateShader(L"Shaders/Bin/render_uniform_cube.glsl.spv",
		                                                   RenderInterface::ShaderType::VertexShader);
		std::shared_ptr FragmentShader = Device.CreateShader(L"Shaders/Bin/load_equirectangular_frag.glsl.spv",
		                                                     RenderInterface::ShaderType::FragmentShader);

		RenderInterface::SamplerDescription SamplerDescription = {};
		SamplerDescription.AddressingModeU = RenderInterface::AddressingMode::Repeat;
		SamplerDescription.AddressingModeV = RenderInterface::AddressingMode::Repeat;
		SamplerDescription.MinificationFilteringMode = RenderInterface::FilteringMode::Nearest;
		SamplerDescription.MagnificationFilteringMode = RenderInterface::FilteringMode::Nearest;
		SamplerDescription.CoordinateSystem = RenderInterface::CoordinateSystem::Normalized;
		SamplerDescription.MipMode = RenderInterface::MipmappingMode::Linear;
		auto EquirectangularTextureSampler = Device.CreateSampler(SamplerDescription);

		RenderInterface::DescriptorBinding TextureBinding = {};
		TextureBinding.Index = 0;
		TextureBinding.DescriptorCount = 1;
		TextureBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		TextureBinding.Type = RenderInterface::DescriptorType::CombinedSampler;
		std::shared_ptr DescriptorLayout = Device.CreateDescriptorSetLayout({ TextureBinding });
		auto DescriptorSet = Renderer::Get().GetDescriptorAllocator().Allocate(*DescriptorLayout);
		DescriptorSet->UpdateWithImageAndSampler(0, 0, EquirectangularTexture.GetDefaultView(),
		                                         *EquirectangularTextureSampler,
		                                         RenderInterface::ImageLayout::ShaderReadOnlyOptimal);

		RenderInterface::PipelineDescription PipelineDescription = {};
		PipelineDescription.PushConstants = { { RenderInterface::ShaderType::VertexShader, 0, sizeof(Mat4) } };
		PipelineDescription.ShaderStages = { VertexShader.get(), FragmentShader.get() };
		PipelineDescription.DescriptorLayouts = { DescriptorLayout.get() };
		PipelineDescription.InputAssembler.Topology = RenderInterface::TopologyType::TriangleList;
		PipelineDescription.Viewport.Dimensions = Image->GetSize();
		PipelineDescription.Viewport.Origin = { 0, 0 };
		PipelineDescription.Rasterizer.Cull = RenderInterface::CullMode::Back;
		PipelineDescription.Rasterizer.Direction = RenderInterface::FaceDirection::CounterClockwise;
		PipelineDescription.Rasterizer.Fill = RenderInterface::FillMode::Fill;
		PipelineDescription.DepthStencilStage.IsDepthTestEnabled = false;
		PipelineDescription.DepthStencilStage.IsDepthWriteEnabled = false;
		auto Pipeline = Device.CreatePipeline(*RenderPass, PipelineDescription);

		auto RenderingFinishFence = Device.CreateFence();

		constexpr RenderInterface::CubemapSide Sides[6] =
		{
			RenderInterface::CubemapSide::PositiveX, RenderInterface::CubemapSide::NegativeX,
			RenderInterface::CubemapSide::PositiveY, RenderInterface::CubemapSide::NegativeY,
			RenderInterface::CubemapSide::PositiveZ, RenderInterface::CubemapSide::NegativeZ
		};

		for (const auto Side : Sides)
		{
			RenderInterface::ImageViewDescription ViewDescription = {};
			ViewDescription.Aspects = RenderInterface::ImageAspect::Color;
			ViewDescription.BaseMipLevel = 0;
			ViewDescription.MipLevelCount = 1;
			auto CubemapView = Image->CreateCubemapImageView(ViewDescription, Side);

			std::vector<const RenderInterface::ImageView*> RenderTargetAttachments = { CubemapView.get() };
			auto RenderTarget = Device.CreateRenderTarget(*RenderPass, RenderTargetAttachments, Image->GetSize());
			CommandBuffer->BeginRecording();

			RenderInterface::ImageMemoryBarrier CubemapBarrier = {};
			CubemapBarrier.OldLayout = RenderInterface::ImageLayout::Undefined;
			CubemapBarrier.NewLayout = RenderInterface::ImageLayout::ColorAttachmentOptimal;
			CubemapBarrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::None;
			CubemapBarrier.OperationsThatCanStartAfter = RenderInterface::AccessType::ColorAttachmentRead |
				RenderInterface::AccessType::ColorAttachmentWrite;
			CubemapBarrier.BaseMipLevel = 0;
			CubemapBarrier.MipLevelCount = Image->GetMipLevelsCount();
			CubemapBarrier.Side = Side;
			CommandBuffer->InsertImageMemoryBarrier(*Image, CubemapBarrier,
			                                        RenderInterface::PipelineStage::BottomOfPipe,
			                                        RenderInterface::PipelineStage::ColorAttachmentOutput);

			CommandBuffer->BeginRenderPass(*RenderPass, *RenderTarget, { { 0.0f, 0.0f, 0.0f, 0.0f } });
			CommandBuffer->BindPipeline(*Pipeline);
			CommandBuffer->BindDescriptorSet(*DescriptorSet, *Pipeline, 0);

			Mat4 ViewMatrix;
			switch (Side)
			{
			case RenderInterface::CubemapSide::PositiveX:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
				break;
			case RenderInterface::CubemapSide::NegativeX:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
				break;
			case RenderInterface::CubemapSide::PositiveY:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
				break;
			case RenderInterface::CubemapSide::NegativeY:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
				break;
			case RenderInterface::CubemapSide::PositiveZ:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f));
				break;
			case RenderInterface::CubemapSide::NegativeZ:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f));
				break;
			default:
				HERMES_ASSERT(false);
			}
			auto AspectRatio = static_cast<float>(CubemapDimensions.X) / static_cast<float>(CubemapDimensions.Y);
			auto ProjectionMatrix = Mat4::Perspective(Math::Pi / 2, AspectRatio, 0.1f, 10.0f);
			auto ViewProjection = ProjectionMatrix * ViewMatrix;

			CommandBuffer->UploadPushConstants(*Pipeline, RenderInterface::ShaderType::VertexShader, &ViewProjection,
			                                   sizeof(ViewProjection), 0);
			// NOTE : 36 vertices of cube should be encoded in the vertex shader
			CommandBuffer->Draw(36, 1, 0, 0);
			CommandBuffer->EndRenderPass();

			// NOTE : if we do not have to generate mip maps we have to manually perform layout transition
			if (!EnableMipMaps)
			{
				RenderInterface::ImageMemoryBarrier Barrier = {};
				Barrier.OldLayout = RenderInterface::ImageLayout::ColorAttachmentOptimal;
				Barrier.NewLayout = RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
				Barrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::ColorAttachmentWrite;
				Barrier.OperationsThatCanStartAfter = RenderInterface::AccessType::MemoryRead |
					RenderInterface::AccessType::MemoryWrite;
				Barrier.BaseMipLevel = 0;
				Barrier.MipLevelCount = 1;
				Barrier.Side = Side;
				CommandBuffer->InsertImageMemoryBarrier(*Image, Barrier,
				                                        RenderInterface::PipelineStage::ColorAttachmentOutput,
				                                        RenderInterface::PipelineStage::TopOfPipe);
			}

			CommandBuffer->EndRecording();

			Queue.SubmitCommandBuffer(*CommandBuffer, RenderingFinishFence.get());
			// TODO : try to draw all 6 sides simultaneously
			RenderingFinishFence->Wait(UINT64_MAX);
			RenderingFinishFence->Reset();

			if (EnableMipMaps)
			{
				GPUInteractionUtilities::GenerateMipMaps(*Image, RenderInterface::ImageLayout::ColorAttachmentOptimal,
				                                         RenderInterface::ImageLayout::ShaderReadOnlyOptimal, Side);
			}
		}

		DataUploadFinished = true;
		DefaultView = Image->CreateDefaultImageView();
	}
}
