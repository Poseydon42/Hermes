#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Image.h"

namespace Hermes
{
	class ImageAsset;

	class HERMES_API Texture
	{
	public:
		static std::shared_ptr<Texture> CreateFromAsset(std::weak_ptr<ImageAsset> Source);

		const RenderInterface::Image& GetRawImage() const;

		Vec2ui GetDimensions() const;

		bool IsReady() const;
	private:
		explicit Texture(std::weak_ptr<ImageAsset> Source);

		std::weak_ptr<ImageAsset> Asset;

		bool DataUploadFinished;
		std::shared_ptr<RenderInterface::Image> Image;
		Vec2ui Dimensions;
	};
}
