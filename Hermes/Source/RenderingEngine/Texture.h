﻿#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Image.h"

namespace Hermes
{
	class ImageAsset;

	class HERMES_API Texture
	{
	public:
		static std::shared_ptr<Texture> CreateFromAsset(const ImageAsset& Source);

		const RenderInterface::Image& GetRawImage() const;

		Vec2ui GetDimensions() const;

		uint32 GetMipLevelsCount() const;

		bool IsReady() const;
	private:
		explicit Texture(const ImageAsset& Source);

		bool DataUploadFinished;
		std::shared_ptr<RenderInterface::Image> Image;
		Vec2ui Dimensions;
	};
}
