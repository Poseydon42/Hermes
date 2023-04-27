#pragma once

#include <unordered_map>

#include "Core/Core.h"
#include "Math/Rect2D.h"
#include "UIEngine/Font.h"
#include "Vulkan/Image.h"

namespace Hermes
{
	class HERMES_API FontPack
	{
	public:
		explicit FontPack(AssetHandle<UI::Font> InFont);

		void UpdatePack(std::span<const uint32> RequiredGlyphs);

		Rect2Dui GetGlyphCoordinates(uint32 GlyphIndex) const;

		const Vulkan::ImageView& GetImage() const;

	private:
		AssetHandle<UI::Font> Font;

		static constexpr VkFormat FontPackImageFormat = VK_FORMAT_R8_UNORM;
		std::unique_ptr<Vulkan::Image> FontPackImage;
		std::unique_ptr<Vulkan::ImageView> FontPackImageView;

		std::unordered_map<uint32, Rect2Dui> GlyphCoordinates;
	};
}
