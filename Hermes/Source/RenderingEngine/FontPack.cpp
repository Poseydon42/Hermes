#include "FontPack.h"

#include "Core/Profiling.h"
#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"

namespace Hermes
{
	FontPack::FontPack(AssetHandle<UI::Font> InFont)
		: Font(std::move(InFont))
	{
	}

	void FontPack::UpdatePack(std::span<const uint32> RequiredGlyphs)
	{
		HERMES_PROFILE_FUNC();

		std::unordered_map<uint32, UI::RenderedGlyph> RenderedGlyphs;
		Vec2ui MaxGlyphDimensions = { 0 };

		for (auto GlyphIndex : RequiredGlyphs)
		{
			if (RenderedGlyphs.contains(GlyphIndex))
				continue;

			auto MaybeGlyph = Font->RenderGlyph(GlyphIndex);
			HERMES_ASSERT(MaybeGlyph.has_value()); // FIXME: do not crash here, but rather replace missing glyph with empty glyph
			auto Glyph = std::move(MaybeGlyph.value());

			MaxGlyphDimensions.X = Math::Max(MaxGlyphDimensions.X, Glyph.Dimensions.X);
			MaxGlyphDimensions.Y = Math::Max(MaxGlyphDimensions.Y, Glyph.Dimensions.Y);

			RenderedGlyphs[GlyphIndex] = std::move(Glyph);
		}

		auto GlyphCountInRow = static_cast<uint32>(Math::Sqrt(RenderedGlyphs.size()));
		auto RowCount = static_cast<uint32>((RenderedGlyphs.size() + GlyphCountInRow - 1) / GlyphCountInRow);
		HERMES_ASSERT(GlyphCountInRow * RowCount >= RenderedGlyphs.size());

		static constexpr uint32 HalfGlyphPadding = 2;
		Vec2ui FullGlyphSlotSize = MaxGlyphDimensions + HalfGlyphPadding * 2;
		Vec2ui ImageDimensions = FullGlyphSlotSize * Vec2ui(GlyphCountInRow, RowCount);

		FontPackImage = Renderer::GetDevice().CreateImage(ImageDimensions, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, FontPackImageFormat, 1);
		FontPackImageView = FontPackImage->CreateDefaultImageView();

		uint32 CurrentGlyphIndex = 0;
		for (const auto& [GlyphIndex, Glyph] : RenderedGlyphs)
		{
			// NOTE: some characters might not have a bitmap (e.g. whitespace)
			if (Glyph.Dimensions.X == 0 || Glyph.Dimensions.Y == 0)
				continue;

			auto GlyphXIndex = CurrentGlyphIndex % GlyphCountInRow;
			auto GlyphYIndex = CurrentGlyphIndex / GlyphCountInRow;

			Vec2ui GlyphSlotPosition = FullGlyphSlotSize * Vec2ui(GlyphXIndex, GlyphYIndex);
			auto GlyphPosition = GlyphSlotPosition + HalfGlyphPadding;

			auto CurrentImageLayout = (CurrentGlyphIndex == 0 ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			GPUInteractionUtilities::UploadDataToGPUImage(Glyph.Bitmap.data(), GlyphPosition, Glyph.Dimensions, 1, 0, *FontPackImage, CurrentImageLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			
			GlyphCoordinates[GlyphIndex] = Rect2Dui(GlyphPosition, GlyphPosition + Glyph.Dimensions);

			CurrentGlyphIndex++;
		}
	}

	Rect2Dui FontPack::GetGlyphCoordinates(uint32 GlyphIndex) const
	{
		if (GlyphCoordinates.contains(GlyphIndex))
			return GlyphCoordinates.at(GlyphIndex);
		return {};
	}

	const Vulkan::ImageView& FontPack::GetImage() const
	{
		HERMES_ASSERT(FontPackImageView);
		return *FontPackImageView;
	}
}
