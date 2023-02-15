#include "Texture.h"

#include "Logging/Logger.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "AssetSystem/ImageAsset.h"
#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Image.h"
#include "Vulkan/Device.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Queue.h"
#include "Vulkan/Sampler.h"

namespace Hermes
{
	static VkFormat ChooseFormatFromImageType(ImageFormat Format, size_t BytesPerChannel, bool IsSRGB)
	{
		using FormatCombinationHashType = size_t;
		constexpr auto ComputeHashForFormat = [](ImageFormat Format, uint8 BytesPerChannel,
		                               bool IsSRGB) -> FormatCombinationHashType
		{
			return (static_cast<uint8>(Format) << 16) | (BytesPerChannel << 8) | static_cast<uint8>(IsSRGB);
		};

		static const std::unordered_map<FormatCombinationHashType, VkFormat> FormatCombinations =
		{
			{ ComputeHashForFormat(ImageFormat::R, 1, true), VK_FORMAT_R8_SRGB },
			{ ComputeHashForFormat(ImageFormat::R, 1, false), VK_FORMAT_R8_UNORM },
			{ ComputeHashForFormat(ImageFormat::R, 2, false), VK_FORMAT_R16_UNORM },
			{ ComputeHashForFormat(ImageFormat::RG, 1, true), VK_FORMAT_R8G8_SRGB },
			{ ComputeHashForFormat(ImageFormat::RG, 1, false), VK_FORMAT_R8G8_UNORM },
			{ ComputeHashForFormat(ImageFormat::RG, 2, false), VK_FORMAT_R16G16_UNORM },

			{ ComputeHashForFormat(ImageFormat::RA, 1, true), VK_FORMAT_R8G8_SRGB },
			{ ComputeHashForFormat(ImageFormat::RA, 1, false), VK_FORMAT_R8G8_UNORM },
			{ ComputeHashForFormat(ImageFormat::RA, 2, false), VK_FORMAT_R16G16_UNORM },

			{ ComputeHashForFormat(ImageFormat::RGBA, 1, true), VK_FORMAT_R8G8B8A8_SRGB },
			{ ComputeHashForFormat(ImageFormat::RGBA, 1, false), VK_FORMAT_R8G8B8A8_UNORM },
			{ ComputeHashForFormat(ImageFormat::RGBA, 2, false), VK_FORMAT_R16G16B16A16_UNORM },

			{ ComputeHashForFormat(ImageFormat::RGBX, 1, true), VK_FORMAT_R8G8B8A8_SRGB },
			{ ComputeHashForFormat(ImageFormat::RGBX, 1, false), VK_FORMAT_R8G8B8A8_UNORM },
			{ ComputeHashForFormat(ImageFormat::RGBX, 2, false), VK_FORMAT_R16G16B16A16_UNORM },

			{ ComputeHashForFormat(ImageFormat::HDR, 4, false), VK_FORMAT_R32G32B32A32_SFLOAT }
		};

		auto It = FormatCombinations.find(ComputeHashForFormat(Format, static_cast<uint8>(BytesPerChannel), IsSRGB));
		if (It == FormatCombinations.end())
		{
			HERMES_LOG_ERROR("Cannot choose data format for the folowing image properties: image format %hhu; %llu bytes per channel; %s color space",
			                 static_cast<uint8>(Format), BytesPerChannel, (IsSRGB ? "sRGB" : "linear"));
			return VK_FORMAT_UNDEFINED;
		}
		return It->second;
	}

	std::unique_ptr<Texture> Texture::CreateFromAsset(const ImageAsset& Source, bool UseAsSRGB, bool EnableMipMaps)
	{
		return std::unique_ptr<Texture>(new Texture(Source, UseAsSRGB, EnableMipMaps));
	}

	const Vulkan::Image& Texture::GetRawImage() const
	{
		HERMES_ASSERT(IsReady());
		return *Image;
	}

	const Vulkan::ImageView& Texture::GetDefaultView() const
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

	VkFormat Texture::GetDataFormat() const
	{
		if (!Image)
			return VK_FORMAT_UNDEFINED;
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
		                                                      VK_IMAGE_USAGE_SAMPLED_BIT |
		                                                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		                                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT, Format, MipLevelCount);

		// NOTE : normally, after loading image we will move it into
		// transfer source layout for further blitting to generate mip
		// maps and transition to shader read only layout will be done
		// in GPUInteractionUtilities::GenerateMipMaps(). However, if
		// user does not want to generate mip maps or if they are
		// precomputed(so we only upload them) then we have to perform
		// transition to shader read only layout immediately ourselves
		VkImageLayout LayoutAfterLoad;
		if (EnableMipMaps && !Source.HasPrecomputedMips())
		{
			LayoutAfterLoad = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		}
		else
		{
			LayoutAfterLoad = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		GPUInteractionUtilities::UploadDataToGPUImage(Source.GetRawData(), { 0, 0 }, Dimensions,
		                                              Source.GetBytesPerPixel(), 0, *Image,
		                                              VK_IMAGE_LAYOUT_UNDEFINED, LayoutAfterLoad);

		if (EnableMipMaps)
		{
			if (Source.HasPrecomputedMips())
			{
				for (uint8 MipLevel = 1; MipLevel < Source.GetMipLevelCount(); MipLevel++)
				{
					GPUInteractionUtilities::UploadDataToGPUImage(Source.GetRawData(MipLevel), { 0, 0 },
					                                              Source.GetDimensions(MipLevel),
					                                              Source.GetBytesPerPixel(), MipLevel, *Image,
					                                              VK_IMAGE_LAYOUT_UNDEFINED,
					                                              LayoutAfterLoad);
				}
			}
			else
			{
				GPUInteractionUtilities::GenerateMipMaps(*Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}

		DataUploadFinished = true;

		DefaultView = Image->CreateDefaultImageView();
	}

	std::unique_ptr<CubemapTexture> CubemapTexture::CreateEmpty(Vec2ui InDimensions, VkFormat InFormat,
	                                                            VkImageUsageFlags InUsage, uint32 InMipLevelCount)
	{
		return std::unique_ptr<CubemapTexture>(new CubemapTexture(InDimensions, InFormat, InUsage, InMipLevelCount));
	}

	std::unique_ptr<CubemapTexture> CubemapTexture::CreateFromEquirectangularTexture(
		const Texture& EquirectangularTexture, VkFormat PreferredFormat, bool EnableMipMaps)
	{
		return std::unique_ptr<CubemapTexture>(new CubemapTexture(EquirectangularTexture, PreferredFormat, EnableMipMaps));
	}

	CubemapTexture::CubemapTexture(Vec2ui InDimensions, VkFormat InFormat, VkImageUsageFlags InUsage,
	                               uint32 InMipLevelCount)
	{
		Image = Renderer::Get().GetActiveDevice().CreateCubemap(InDimensions, InUsage, InFormat, InMipLevelCount);

		Dimensions = InDimensions;
		DataUploadFinished = true;

		DefaultView = Image->CreateDefaultImageView();
	}

	CubemapTexture::CubemapTexture(const Texture& EquirectangularTexture, VkFormat PreferredFormat, bool EnableMipMaps)
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
		                             VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		                             CubemapFormat, MipMapCount);

		auto& Queue = Device.GetQueue(VK_QUEUE_GRAPHICS_BIT);
		auto CommandBuffer = Queue.CreateCommandBuffer(true);

		VkAttachmentDescription OutputAttachment = {};
		OutputAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		OutputAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		OutputAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		OutputAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		OutputAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		OutputAttachment.format = CubemapFormat;
		OutputAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		OutputAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		auto RenderPass = Device.CreateRenderPass({
			                                          std::make_pair(OutputAttachment, Vulkan::AttachmentType::Color) 
		                                          });

		std::unique_ptr VertexShader = Device.CreateShader("/Shaders/Bin/render_uniform_cube.glsl.spv",
		                                                   VK_SHADER_STAGE_VERTEX_BIT);
		std::unique_ptr FragmentShader = Device.CreateShader("/Shaders/Bin/load_equirectangular_frag.glsl.spv",
		                                                     VK_SHADER_STAGE_FRAGMENT_BIT);

		Vulkan::SamplerDescription SamplerDescription = {};
		SamplerDescription.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		SamplerDescription.MinificationFilter = VK_FILTER_NEAREST;
		SamplerDescription.MagnificationFilter = VK_FILTER_NEAREST;
		SamplerDescription.CoordinateSystem = Vulkan::CoordinateSystem::Normalized;
		SamplerDescription.MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		auto EquirectangularTextureSampler = Device.CreateSampler(SamplerDescription);

		VkDescriptorSetLayoutBinding TextureBinding = {};
		TextureBinding.binding = 0;
		TextureBinding.descriptorCount = 1;
		TextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		TextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		std::shared_ptr DescriptorLayout = Device.CreateDescriptorSetLayout({ TextureBinding });
		auto DescriptorSet = Renderer::Get().GetDescriptorAllocator().Allocate(*DescriptorLayout);
		DescriptorSet->UpdateWithImageAndSampler(0, 0, EquirectangularTexture.GetDefaultView(),
		                                         *EquirectangularTextureSampler,
		                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		Vulkan::PipelineDescription PipelineDescription = {};
		PipelineDescription.PushConstants = { { VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4) } };
		PipelineDescription.ShaderStages = { VertexShader.get(), FragmentShader.get() };
		PipelineDescription.DescriptorSetLayouts = { DescriptorLayout.get() };
		PipelineDescription.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		PipelineDescription.Viewport.width = static_cast<float>(Image->GetDimensions().X);
		PipelineDescription.Viewport.height = static_cast<float>(Image->GetDimensions().X);
		PipelineDescription.Viewport.x = 0.0f;
		PipelineDescription.Viewport.y = 0.0f;
		PipelineDescription.Viewport.minDepth = 0.0f;
		PipelineDescription.Viewport.maxDepth = 1.0f;
		PipelineDescription.Scissor.offset = { 0, 0 };
		PipelineDescription.Scissor.extent = { Image->GetDimensions().X, Image->GetDimensions().Y };
		PipelineDescription.CullMode = VK_CULL_MODE_BACK_BIT;
		PipelineDescription.FaceDirection = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		PipelineDescription.PolygonMode = VK_POLYGON_MODE_FILL;
		PipelineDescription.IsDepthTestEnabled = false;
		PipelineDescription.IsDepthWriteEnabled = false;
		auto Pipeline = Device.CreatePipeline(*RenderPass, PipelineDescription);

		auto RenderingFinishFence = Device.CreateFence();

		constexpr Vulkan::CubemapSide Sides[6] =
		{
			Vulkan::CubemapSide::PositiveX, Vulkan::CubemapSide::NegativeX,
			Vulkan::CubemapSide::PositiveY, Vulkan::CubemapSide::NegativeY,
			Vulkan::CubemapSide::PositiveZ, Vulkan::CubemapSide::NegativeZ
		};

		for (const auto Side : Sides)
		{
			VkImageSubresourceRange Subresource = {};
			Subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			Subresource.baseMipLevel = 0;
			Subresource.levelCount = 1;
			Subresource.baseArrayLayer = CubemapSideToArrayLayer(Side);
			Subresource.layerCount = 1;
			auto CubemapView = Image->CreateImageView(Subresource);

			std::vector<const Vulkan::ImageView*> FramebufferAttachments = { CubemapView.get() };
			auto Framebuffer = Device.CreateFramebuffer(*RenderPass, FramebufferAttachments, Image->GetDimensions());
			CommandBuffer->BeginRecording();

			VkImageMemoryBarrier CubemapBarrier = {};
			CubemapBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			CubemapBarrier.image = Image->GetImage();
			CubemapBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			CubemapBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			CubemapBarrier.srcAccessMask = VK_ACCESS_NONE;
			CubemapBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			CubemapBarrier.subresourceRange.baseMipLevel = 0;
			CubemapBarrier.subresourceRange.levelCount = Image->GetMipLevelsCount();
			CubemapBarrier.subresourceRange.baseArrayLayer = CubemapSideToArrayLayer(Side);
			CubemapBarrier.subresourceRange.layerCount = 1;
			CubemapBarrier.subresourceRange.aspectMask = Image->GetFullAspectMask();
			CommandBuffer->InsertImageMemoryBarrier(CubemapBarrier, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			                                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

			VkClearValue ClearValue = { 0.0f, 0.0f, 0.0f, 0.0f };
			CommandBuffer->BeginRenderPass(*RenderPass, *Framebuffer, { &ClearValue, 1 });
			CommandBuffer->BindPipeline(*Pipeline);
			CommandBuffer->BindDescriptorSet(*DescriptorSet, *Pipeline, 0);

			Mat4 ViewMatrix;
			switch (Side)
			{
			case Vulkan::CubemapSide::PositiveX:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
				break;
			case Vulkan::CubemapSide::NegativeX:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
				break;
			case Vulkan::CubemapSide::PositiveY:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
				break;
			case Vulkan::CubemapSide::NegativeY:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
				break;
			case Vulkan::CubemapSide::PositiveZ:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f));
				break;
			case Vulkan::CubemapSide::NegativeZ:
				ViewMatrix = Mat4::LookAt(Vec3(0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f));
				break;
			default:
				HERMES_ASSERT(false);
			}
			auto AspectRatio = static_cast<float>(CubemapDimensions.X) / static_cast<float>(CubemapDimensions.Y);
			auto ProjectionMatrix = Mat4::Perspective(Math::Pi / 2, AspectRatio, 0.1f, 10.0f);
			auto ViewProjection = ProjectionMatrix * ViewMatrix;

			CommandBuffer->UploadPushConstants(*Pipeline, VK_SHADER_STAGE_VERTEX_BIT, &ViewProjection,
			                                   sizeof(ViewProjection), 0);
			// NOTE : 36 vertices of cube should be encoded in the vertex shader
			CommandBuffer->Draw(36, 1, 0, 0);
			CommandBuffer->EndRenderPass();

			// NOTE : if we do not have to generate mip maps we have to manually perform layout transition
			if (!EnableMipMaps)
			{
				VkImageMemoryBarrier Barrier = {};
				Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				Barrier.image = Image->GetImage();
				Barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				Barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				Barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				Barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
				Barrier.subresourceRange.baseMipLevel = 0;
				Barrier.subresourceRange.levelCount = 1;
				Barrier.subresourceRange.baseArrayLayer = CubemapSideToArrayLayer(Side);
				Barrier.subresourceRange.layerCount = 1;
				Barrier.subresourceRange.aspectMask = Image->GetFullAspectMask();
				CommandBuffer->InsertImageMemoryBarrier(Barrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				                                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
			}

			CommandBuffer->EndRecording();

			Queue.SubmitCommandBuffer(*CommandBuffer, RenderingFinishFence.get());
			// TODO : try to draw all 6 sides simultaneously
			RenderingFinishFence->Wait(UINT64_MAX);
			RenderingFinishFence->Reset();

			if (EnableMipMaps)
			{
				GPUInteractionUtilities::GenerateMipMaps(*Image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, Side);
			}
		}

		DataUploadFinished = true;
		DefaultView = Image->CreateDefaultImageView();
	}
}
