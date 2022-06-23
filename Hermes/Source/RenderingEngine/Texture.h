#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Math/Math.h"
#include "RenderInterface/GenericRenderInterface/Image.h"

namespace Hermes
{
	class ImageAsset;

	class HERMES_API Texture
	{
		MAKE_NON_COPYABLE(Texture)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(Texture)
		ADD_DEFAULT_DESTRUCTOR(Texture)

	public:
		static std::shared_ptr<Texture> CreateFromAsset(const ImageAsset& Source, bool EnableMipMaps = true);

		const RenderInterface::Image& GetRawImage() const;

		const RenderInterface::ImageView& GetDefaultView() const;

		Vec2ui GetDimensions() const;

		uint32 GetMipLevelsCount() const;

		bool IsReady() const;
	private:
		explicit Texture(const ImageAsset& Source, bool EnableMipMaps = true);

		bool DataUploadFinished;
		std::unique_ptr<RenderInterface::Image> Image;
		std::unique_ptr<RenderInterface::ImageView> DefaultView;
		Vec2ui Dimensions;
	};
}
