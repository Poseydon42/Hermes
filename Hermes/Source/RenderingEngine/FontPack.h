#pragma once

#include <unordered_map>
#include <unordered_set>

#include "Core/Core.h"
#include "Math/Rect2D.h"
#include "UIEngine/Font.h"
#include "Vulkan/Image.h"

namespace Hermes
{
	class HERMES_API FontPack
	{
	public:
		void RequestGlyph(const UI::Font& Font, uint32 GlyphIndex, uint32 FontSize);

		void Repack();

		Rect2Dui GetGlyphCoordinates(const UI::Font& Font, uint32 GlyphIndex, uint32 FontSize) const;

		const Vulkan::ImageView& GetImage() const;

	private:
		static constexpr VkFormat FontPackImageFormat = VK_FORMAT_R8_UNORM;
		std::unique_ptr<Vulkan::Image> FontPackImage;
		std::unique_ptr<Vulkan::ImageView> FontPackImageView;

		struct GlyphDescription
		{
			uint32 FontID;
			uint32 FontSize;
			uint32 GlyphIndex;

			bool operator==(const GlyphDescription& Other) const;
		};
		struct GlyphDescriptionHasher
		{
			size_t operator()(const GlyphDescription& Value) const;
		};
		std::unordered_map<GlyphDescription, UI::RenderedGlyph, GlyphDescriptionHasher> GlyphsPendingPacking;
		bool NeedsRepacking = false;

		std::unordered_map<GlyphDescription, Rect2Dui, GlyphDescriptionHasher> GlyphCoordinates;
	};
}
