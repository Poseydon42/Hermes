﻿#include "Texture.h"

#include "AssetSystem/ImageAsset.h"
#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"

namespace Hermes
{
	static RenderInterface::DataFormat ChooseFormatFromImageType(ImageFormat Format)
	{
		switch (Format)
		{
		case ImageFormat::R8:
			return RenderInterface::DataFormat::R8UnsignedNormalized;
		case ImageFormat::R16:
			return RenderInterface::DataFormat::R16UnsignedNormalized;
		case ImageFormat::R32:
			return RenderInterface::DataFormat::R32UnsignedInteger;
		case ImageFormat::R8G8:
			return RenderInterface::DataFormat::R8G8UnsignedNormalized;
		case ImageFormat::R16G16:
			return RenderInterface::DataFormat::R16G16UnsignedNormalized;
		case ImageFormat::B8G8R8A8:
		case ImageFormat::B8G8R8X8:
			return RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized;
		default:
			HERMES_ASSERT(false);
			return static_cast<RenderInterface::DataFormat>(0);
		}
	}

	std::shared_ptr<Texture> Texture::CreateFromAsset(std::weak_ptr<ImageAsset> Source)
	{
		return std::shared_ptr<Texture>(new Texture(std::move(Source)));
	}

	const RenderInterface::Image& Texture::GetRawImage() const
	{
		HERMES_ASSERT(IsReady());
		return *Image;
	}

	Vec2ui Texture::GetDimensions() const
	{
		return Dimensions;
	}

	uint32 Texture::GetMipLevelsCount() const
	{
		return Image->GetMipLevelsCount();
	}

	bool Texture::IsReady() const
	{
		return DataUploadFinished && Image != nullptr && Dimensions.LengthSq() > 0;
	}

	Texture::Texture(std::weak_ptr<ImageAsset> Source)
		: DataUploadFinished(false)
		, Dimensions(0)
	{
		auto LockedAsset = Source.lock();
		if (!LockedAsset)
			return;
		
		Dimensions = LockedAsset->GetDimensions();
		uint32 BiggestDimension = Math::Max(Dimensions.X, Dimensions.Y);
		HERMES_ASSERT(BiggestDimension > 0);
		uint32 MipLevelCount = Math::FloorLog2(BiggestDimension);
		Image = Renderer::Get().GetActiveDevice().CreateImage(
			Dimensions, 
			RenderInterface::ImageUsageType::Sampled | RenderInterface::ImageUsageType::CopyDestination | RenderInterface::ImageUsageType::CopySource,
			ChooseFormatFromImageType(LockedAsset->GetImageFormat()), MipLevelCount + 1, RenderInterface::ImageLayout::Undefined);

		GPUInteractionUtilities::UploadDataToGPUImage(
			LockedAsset->GetRawData(), { 0, 0 }, Dimensions,
			LockedAsset->GetBitsPerPixel() / 8, 0, *Image, 
			RenderInterface::ImageLayout::Undefined, RenderInterface::ImageLayout::TransferSourceOptimal);

		GPUInteractionUtilities::GenerateMipMaps(
			*Image, RenderInterface::ImageLayout::TransferSourceOptimal, RenderInterface::ImageLayout::ShaderReadOnlyOptimal);

		DataUploadFinished = true;
	}
}
