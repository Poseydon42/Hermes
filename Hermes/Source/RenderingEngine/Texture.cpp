﻿#include "Texture.h"

#include "AssetSystem/AssetLoader.h"
#include "Logging/Logger.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Device.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Framebuffer.h"
#include "Vulkan/Image.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Queue.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Sampler.h"

namespace Hermes
{
	HERMES_ADD_BINARY_ASSET_LOADER(Texture2D, Texture2D);

	static VkFormat ChooseFormatFromImageType(ImageFormat Format, size_t BytesPerChannel)
	{
		using FormatCombinationHashType = size_t;
		constexpr auto ComputeHashForFormat = [](ImageFormat Format, uint8 BytesPerChannel) -> FormatCombinationHashType
		{
			return (static_cast<uint8>(Format) << 16) | (BytesPerChannel << 8);
		};

		static const std::unordered_map<FormatCombinationHashType, VkFormat> FormatCombinations =
		{
			{ ComputeHashForFormat(ImageFormat::R, 1), VK_FORMAT_R8_UNORM },
			{ ComputeHashForFormat(ImageFormat::R, 2), VK_FORMAT_R16_UNORM },
			{ ComputeHashForFormat(ImageFormat::RG, 1), VK_FORMAT_R8G8_UNORM },
			{ ComputeHashForFormat(ImageFormat::RG, 2), VK_FORMAT_R16G16_UNORM },

			{ ComputeHashForFormat(ImageFormat::RA, 1), VK_FORMAT_R8G8_UNORM },
			{ ComputeHashForFormat(ImageFormat::RA, 2), VK_FORMAT_R16G16_UNORM },

			{ ComputeHashForFormat(ImageFormat::RGBA, 1), VK_FORMAT_R8G8B8A8_UNORM },
			{ ComputeHashForFormat(ImageFormat::RGBA, 2), VK_FORMAT_R16G16B16A16_UNORM },

			{ ComputeHashForFormat(ImageFormat::RGBX, 1), VK_FORMAT_R8G8B8A8_UNORM },
			{ ComputeHashForFormat(ImageFormat::RGBX, 2), VK_FORMAT_R16G16B16A16_UNORM },

			{ ComputeHashForFormat(ImageFormat::HDR, 4), VK_FORMAT_R32G32B32A32_SFLOAT }
		};

		auto It = FormatCombinations.find(ComputeHashForFormat(Format, static_cast<uint8>(BytesPerChannel)));
		if (It == FormatCombinations.end())
		{
			HERMES_LOG_ERROR("Cannot choose data format for the folowing image properties: image format %hhu; %llu bytes per channel", static_cast<uint8>(Format), BytesPerChannel);
			return VK_FORMAT_UNDEFINED;
		}
		return It->second;
	}

	static size_t GetBytesPerPixel(ImageFormat Format, size_t BytesPerChannel)
	{
		switch (Format)
		{
		case ImageFormat::R:
			return 1 * BytesPerChannel;
		case ImageFormat::RG:
		case ImageFormat::RA:
			return 2 * BytesPerChannel;
		case ImageFormat::RGBA:
		case ImageFormat::RGBX:
		case ImageFormat::HDR:
			return 4 * BytesPerChannel;
		default:
			HERMES_ASSERT(false);
		}
	}

	static VkFormat GetCorrespondingSRGBFormat(VkFormat BaseFormat)
	{
		switch (BaseFormat)
		{
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R8_SNORM:
		case VK_FORMAT_R8_SSCALED:
		case VK_FORMAT_R8_UINT:
		case VK_FORMAT_R8_UNORM:
		case VK_FORMAT_R8_USCALED:
			return VK_FORMAT_R8_SRGB;
		case VK_FORMAT_R8G8_SINT:
		case VK_FORMAT_R8G8_SNORM:
		case VK_FORMAT_R8G8_SSCALED:
		case VK_FORMAT_R8G8_UINT:
		case VK_FORMAT_R8G8_UNORM:
		case VK_FORMAT_R8G8_USCALED:
			return VK_FORMAT_R8G8_SRGB;
		case VK_FORMAT_R8G8B8_SINT:
		case VK_FORMAT_R8G8B8_SNORM:
		case VK_FORMAT_R8G8B8_SSCALED:
		case VK_FORMAT_R8G8B8_UINT:
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_R8G8B8_USCALED:
			return VK_FORMAT_R8G8B8_SRGB;
		case VK_FORMAT_B8G8R8_SINT:
		case VK_FORMAT_B8G8R8_SNORM:
		case VK_FORMAT_B8G8R8_SSCALED:
		case VK_FORMAT_B8G8R8_UINT:
		case VK_FORMAT_B8G8R8_UNORM:
		case VK_FORMAT_B8G8R8_USCALED:
			return VK_FORMAT_B8G8R8_SRGB;
		case VK_FORMAT_R8G8B8A8_SINT:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_R8G8B8A8_SSCALED:
		case VK_FORMAT_R8G8B8A8_UINT:
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_USCALED:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case VK_FORMAT_B8G8R8A8_SINT:
		case VK_FORMAT_B8G8R8A8_SNORM:
		case VK_FORMAT_B8G8R8A8_SSCALED:
		case VK_FORMAT_B8G8R8A8_UINT:
		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_B8G8R8A8_USCALED:
			return VK_FORMAT_B8G8R8A8_SRGB;
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
			return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
		default:
			HERMES_LOG_WARNING("VkFormat %u has no corresponding sRGB format or it is not supported by the engine", static_cast<int32>(BaseFormat));
			return VK_FORMAT_UNDEFINED;
		}
	}

	static size_t NumberOfChannelInImageFormat(ImageFormat Format)
	{
		switch (Format)
		{
		case ImageFormat::R:
			return 1;
		case ImageFormat::RG:
		case ImageFormat::RA:
			return 2;
		case ImageFormat::HDR:
		case ImageFormat::RGBA:
		case ImageFormat::RGBX:
			return 4;
		default:
			HERMES_ASSERT(false);
		}
	}

	static size_t CalculateTotalPixelCount(size_t Width, size_t Height, size_t MipLevelCount)
	{
		size_t Result = 0;
		while (MipLevelCount--)
		{
			Result += Width * Height;
			Width = Math::Max<size_t>(Width / 2, 1);
			Height = Math::Max<size_t>(Height / 2, 1);
		}
		return Result;
	}

	AssetHandle<Texture2D> Texture2D::Create(String Name, Vec2ui Dimensions, ImageFormat Format, size_t BytesPerChannel, const void* Data, MipmapGenerationMode MipmapMode)
	{
		return AssetHandle<Texture2D>(new Texture2D(std::move(Name), Dimensions, Format, BytesPerChannel, Data, MipmapMode));
	}

	AssetHandle<Texture2D> Texture2D::Create(String Name, Vec2ui Dimensions, ImageFormat Format, size_t BytesPerChannel, Vec4 Color)
	{
		return AssetHandle<Texture2D>(new Texture2D(std::move(Name), Dimensions, Format, BytesPerChannel, Color));
	}

	AssetHandle<Asset> Texture2D::Load(String Name, std::span<const uint8> BinaryData)
	{
		const uint8* DataPtr = BinaryData.data();

		const auto* Header = reinterpret_cast<const ImageAssetHeader*>(DataPtr);
		DataPtr += sizeof(*Header);

		size_t BytesPerPixel = NumberOfChannelInImageFormat(Header->Format) * Header->BytesPerChannel;
		size_t TotalBytes = CalculateTotalPixelCount(Header->Width, Header->Height, Header->MipLevelCount) * BytesPerPixel;

		size_t BytesInSource = TotalBytes;
		if (Header->Format == ImageFormat::HDR)
			BytesInSource = BytesInSource / 4 * 3; // We need to insert the alpha channel manually

		std::vector<uint8> ImageData;
		if (Header->Format == ImageFormat::HDR)
		{
			HERMES_ASSERT_LOG(Header->MipLevelCount == 1, "Loading HDR texures with pregenerated mipmaps is not supported");
			
			std::vector<uint8> UnpackedData(TotalBytes, 0x00);
			const auto* SourcePixel = reinterpret_cast<const float*>(DataPtr);
			auto* DestPixel = reinterpret_cast<float*>(UnpackedData.data());

			auto PixelCount = CalculateTotalPixelCount(Header->Width, Header->Height, 1);
			while (PixelCount--)
			{
				*DestPixel++ = *SourcePixel++; // R
				*DestPixel++ = *SourcePixel++; // G
				*DestPixel++ = *SourcePixel++; // B
				*DestPixel++ = 1.0f; // A
			}

			DataPtr = reinterpret_cast<const uint8*>(SourcePixel);

			ImageData = std::move(UnpackedData);
		}
		else
		{
			ImageData = std::vector<uint8>(TotalBytes);
			memcpy(ImageData.data(), DataPtr, TotalBytes);
			DataPtr += TotalBytes;
		}

		HERMES_ASSERT(DataPtr == BinaryData.data() + BinaryData.size());

		// FIXME: let the user choose whether they want to load/generate mipmaps
		auto MipmapGenerationMode = MipmapGenerationMode::Generate;
		if (Header->MipLevelCount > 1)
			MipmapGenerationMode = MipmapGenerationMode::LoadExisting;
		auto Result = Texture2D::Create(String(Name), { Header->Width, Header->Height }, Header->Format, Header->BytesPerChannel, ImageData.data(), MipmapGenerationMode);

		return Result;
	}

	const Vulkan::Image& Texture2D::GetRawImage() const
	{
		return *Image;
	}

	const Vulkan::ImageView& Texture2D::GetView(ColorSpace ColorSpace) const
	{
		HERMES_ASSERT(static_cast<size_t>(ColorSpace) < static_cast<size_t>(ColorSpace::Count_));
		auto& MaybeView = Views[static_cast<size_t>(ColorSpace)];
		if (!MaybeView)
			return CreateView(ColorSpace);
		return *MaybeView;
	}

	Vec2ui Texture2D::GetDimensions() const
	{
		return Image->GetDimensions();
	}

	uint32 Texture2D::GetMipLevelsCount() const
	{
		return Image->GetMipLevelsCount();
	}

	VkFormat Texture2D::GetDataFormat() const
	{
		if (!Image)
			return VK_FORMAT_UNDEFINED;
		return Image->GetDataFormat();
	}

	Texture2D::Texture2D(String Name, Vec2ui Dimensions, ImageFormat Format, size_t BytesPerChannel, const void* Data, MipmapGenerationMode MipmapMode)
		: Asset(std::move(Name), AssetType::Texture2D)
	{
		uint32 BiggestDimension = Math::Max(Dimensions.X, Dimensions.Y);
		HERMES_ASSERT(BiggestDimension > 0);

		uint32 MipLevelCount;
		if (MipmapMode == MipmapGenerationMode::DoNotGenerate)
		{
			MipLevelCount = 0;
		}
		else
		{
			MipLevelCount = Math::FloorLog2(BiggestDimension) + 1;
		}

		auto VulkanFormat = ChooseFormatFromImageType(Format, BytesPerChannel);
		Image = Renderer::GetDevice().CreateImage(Dimensions, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VulkanFormat, MipLevelCount);

		// NOTE : normally, after loading image we will move it into
		// transfer source layout for further blitting to generate mip
		// maps and transition to shader read only layout will be done
		// in GPUInteractionUtilities::GenerateMipMaps(). However, if
		// user does not want to generate mip maps or if they are
		// precomputed(so we only upload them) then we have to perform
		// transition to shader read only layout immediately ourselves
		VkImageLayout LayoutAfterLoad;
		if (MipmapMode == MipmapGenerationMode::Generate)
		{
			LayoutAfterLoad = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		}
		else
		{
			LayoutAfterLoad = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		GPUInteractionUtilities::UploadDataToGPUImage(Data, { 0, 0 }, Dimensions, GetBytesPerPixel(Format, BytesPerChannel), 0, *Image, VK_IMAGE_LAYOUT_UNDEFINED, LayoutAfterLoad);

		if (MipmapMode == MipmapGenerationMode::LoadExisting)
		{
			const auto* CurrentMipData = static_cast<const uint8*>(Data);
			auto CurrentMipLevelDimensions = Dimensions;

			for (uint32 MipLevel = 1; MipLevel < MipLevelCount; MipLevel++)
			{
				CurrentMipData += static_cast<size_t>(CurrentMipLevelDimensions.X) * CurrentMipLevelDimensions.Y * GetBytesPerPixel(Format, BytesPerChannel);
				CurrentMipLevelDimensions /= 2;

				GPUInteractionUtilities::UploadDataToGPUImage(CurrentMipData, { 0, 0 }, CurrentMipLevelDimensions, GetBytesPerPixel(Format, BytesPerChannel), MipLevel, *Image, VK_IMAGE_LAYOUT_UNDEFINED, LayoutAfterLoad);
			}
		}
		else if (MipmapMode == MipmapGenerationMode::Generate)
		{
			GPUInteractionUtilities::GenerateMipMaps(*Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}

	Texture2D::Texture2D(String Name, Vec2ui Dimensions, ImageFormat Format, size_t BytesPerChannel, Vec4 Color)
		: Asset(std::move(Name), AssetType::Texture2D)
	{
		auto VulkanFormat = ChooseFormatFromImageType(Format, BytesPerChannel);
		Image = Renderer::GetDevice().CreateImage(Dimensions, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VulkanFormat, 1);

		auto Range = Image->GetFullSubresourceRange();
		GPUInteractionUtilities::ClearImage(*Image, Color, { &Range, 1 }, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	Vulkan::ImageView& Texture2D::CreateView(ColorSpace ColorSpace) const
	{
		VkFormat Format = VK_FORMAT_UNDEFINED;
		switch (ColorSpace)
		{
		case ColorSpace::Linear:
			Format = Image->GetDataFormat();
			break;
		case ColorSpace::SRGB:
			Format = GetCorrespondingSRGBFormat(Image->GetDataFormat());
			break;
		default:
			HERMES_ASSERT(false);
		}
		VkImageSubresourceRange SubresourceRange = {};
		SubresourceRange.aspectMask = Image->GetFullAspectMask();
		SubresourceRange.baseMipLevel = 0;
		SubresourceRange.levelCount = Image->GetMipLevelsCount();
		SubresourceRange.baseArrayLayer = 0;
		SubresourceRange.layerCount = Image->IsCubemap() ? 6 : 1;
		auto View = Image->IsCubemap() ? Image->CreateCubemapImageView(SubresourceRange, Format) : Image->CreateImageView(SubresourceRange, Format);

		auto& Result = *View;
		Views[static_cast<size_t>(ColorSpace)] = std::move(View);

		return Result;
	}

	std::unique_ptr<TextureCube> TextureCube::CreateEmpty(Vec2ui InDimensions, VkFormat InFormat, VkImageUsageFlags InUsage, uint32 InMipLevelCount)
	{
		return std::unique_ptr<TextureCube>(new TextureCube(InDimensions, InFormat, InUsage, InMipLevelCount));
	}

	std::unique_ptr<TextureCube> TextureCube::CreateFromEquirectangularTexture(const Texture2D& EquirectangularTexture, VkFormat PreferredFormat, bool EnableMipMaps)
	{
		return std::unique_ptr<TextureCube>(new TextureCube(EquirectangularTexture, PreferredFormat, EnableMipMaps));
	}

	const Vulkan::Image& TextureCube::GetRawImage() const
	{
		return *Image;
	}

	const Vulkan::ImageView& TextureCube::GetView() const
	{
		return *View;
	}

	Vec2ui TextureCube::GetDimensions() const
	{
		return Image->GetDimensions();
	}

	uint32 TextureCube::GetMipLevelsCount() const
	{
		return Image->GetMipLevelsCount();
	}

	VkFormat TextureCube::GetDataFormat() const
	{
		return Image->GetDataFormat();
	}

	TextureCube::TextureCube(Vec2ui InDimensions, VkFormat InFormat, VkImageUsageFlags InUsage, uint32 InMipLevelCount)
	{
		Image = Renderer::GetDevice().CreateCubemap(InDimensions, InUsage, InFormat, InMipLevelCount);
		View = Image->CreateDefaultImageView();
	}

	TextureCube::TextureCube(const Texture2D& EquirectangularTexture, VkFormat PreferredFormat, bool EnableMipMaps)
	{
		auto EquirectangularTextureResourceDimensions = EquirectangularTexture.GetDimensions();

		// NOTE : cubemap is 4 times narrower and 2 times shorter
		auto CubemapDimensions = EquirectangularTextureResourceDimensions / Vec2ui { 4, 2 };
		
		auto CubemapFormat = PreferredFormat;

		uint32 MipMapCount = 1;
		if (EnableMipMaps)
			MipMapCount = Math::FloorLog2(Math::Max(CubemapDimensions.X, CubemapDimensions.Y)) + 1;

		auto& Device = Renderer::GetDevice();
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
		auto EquirectangularTextureResourceSampler = Device.CreateSampler(SamplerDescription);

		VkDescriptorSetLayoutBinding TextureResourceBinding = {};
		TextureResourceBinding.binding = 0;
		TextureResourceBinding.descriptorCount = 1;
		TextureResourceBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		TextureResourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		std::shared_ptr DescriptorLayout = Device.CreateDescriptorSetLayout({ TextureResourceBinding });
		auto DescriptorSet = Renderer::GetDescriptorAllocator().Allocate(*DescriptorLayout);
		DescriptorSet->UpdateWithImageAndSampler(0, 0, EquirectangularTexture.GetView(ColorSpace::Linear),
		                                         *EquirectangularTextureResourceSampler,
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

		View = Image->CreateDefaultImageView();
	}
}
