#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/Math.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	class ImageAsset;

	class HERMES_API Texture
	{
	public:
		static std::shared_ptr<Texture> CreateFromAsset(const ImageAsset& Source, bool EnableMipMaps = true);

		const RenderInterface::Image& GetRawImage() const;

		Vec2ui GetDimensions() const;

		uint32 GetMipLevelsCount() const;

		bool IsReady() const;
	private:
		explicit Texture(const ImageAsset& Source, bool EnableMipMaps = true);

		bool DataUploadFinished;
		std::shared_ptr<RenderInterface::Image> Image;
		Vec2ui Dimensions;
	};
}
