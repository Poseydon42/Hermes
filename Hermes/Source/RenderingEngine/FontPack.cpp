#include "FontPack.h"

#include "Core/Profiling.h"
#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"

namespace Hermes
{
	void FontPack::RequestGlyph(const UI::Font& Font, uint32 GlyphIndex, uint32 FontSize)
	{
		GlyphDescription Description = { Font.GetUniqueID(), FontSize, GlyphIndex };
		if (GlyphsPendingPacking.contains(Description))
			return;

		auto MaybeRenderedGlyph = Font.RenderGlyph(GlyphIndex, FontSize);
		HERMES_ASSERT(MaybeRenderedGlyph.has_value());

		GlyphsPendingPacking[Description] = std::move(MaybeRenderedGlyph.value());
		NeedsRepacking = true;
	}

	void FontPack::Repack()
	{
		if (!NeedsRepacking)
			return;

		HERMES_PROFILE_FUNC();

		Vec2ui MaxGlyphDimensions = { 0 };
		for (const auto& [Description, Glyph] : GlyphsPendingPacking)
		{
			MaxGlyphDimensions.X = Math::Max(MaxGlyphDimensions.X, Glyph.Dimensions.X);
			MaxGlyphDimensions.Y = Math::Max(MaxGlyphDimensions.Y, Glyph.Dimensions.Y);
		}

		auto GlyphCountInRow = static_cast<uint32>(Math::Sqrt(GlyphsPendingPacking.size()));
		auto RowCount = static_cast<uint32>((GlyphsPendingPacking.size() + GlyphCountInRow - 1) / GlyphCountInRow);
		HERMES_ASSERT(GlyphCountInRow * RowCount >= GlyphsPendingPacking.size());

		static constexpr uint32 HalfGlyphPadding = 2;
		Vec2ui FullGlyphSlotSize = MaxGlyphDimensions + HalfGlyphPadding * 2;
		Vec2ui ImageDimensions = FullGlyphSlotSize * Vec2ui(GlyphCountInRow, RowCount);

		FontPackImage = Renderer::GetDevice().CreateImage(ImageDimensions, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, FontPackImageFormat, 1);
		FontPackImageView = FontPackImage->CreateDefaultImageView();

		auto Range = FontPackImage->GetFullSubresourceRange();
		GPUInteractionUtilities::ClearImage(*FontPackImage, Vec4(0.0f), { &Range, 1 }, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		uint32 CurrentGlyphIndex = 0;
		for (const auto& [GlyphDescription, Glyph] : GlyphsPendingPacking)
		{
			// NOTE: some characters might not have a bitmap (e.g. whitespace)
			if (Glyph.Dimensions.X == 0 || Glyph.Dimensions.Y == 0)
				continue;

			auto GlyphXIndex = CurrentGlyphIndex % GlyphCountInRow;
			auto GlyphYIndex = CurrentGlyphIndex / GlyphCountInRow;

			Vec2ui GlyphSlotPosition = FullGlyphSlotSize * Vec2ui(GlyphXIndex, GlyphYIndex);
			auto GlyphPosition = GlyphSlotPosition + HalfGlyphPadding;

			GPUInteractionUtilities::UploadDataToGPUImage(Glyph.Bitmap.data(), GlyphPosition, Glyph.Dimensions, 1, 0, *FontPackImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			GlyphCoordinates[GlyphDescription] = Rect2Dui(GlyphPosition, GlyphPosition + Glyph.Dimensions);

			CurrentGlyphIndex++;
		}

		GPUInteractionUtilities::ChangeImageLayout(*FontPackImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		NeedsRepacking = false;
	}

	Rect2Dui FontPack::GetGlyphCoordinates(const UI::Font& Font, uint32 GlyphIndex, uint32 FontSize) const
	{
		HERMES_ASSERT(!NeedsRepacking);

		GlyphDescription Description = { Font.GetUniqueID(), FontSize, GlyphIndex };
		if (GlyphCoordinates.contains(Description))
			return GlyphCoordinates.at(Description);
		return {};
	}
	
	const Vulkan::ImageView& FontPack::GetImage() const
	{
		HERMES_ASSERT(FontPackImageView);
		return *FontPackImageView;
	}

	bool FontPack::GlyphDescription::operator==(const GlyphDescription& Other) const
	{
		return (FontID == Other.FontID && GlyphIndex == Other.GlyphIndex && FontSize == Other.FontSize);
	}

	size_t FontPack::GlyphDescriptionHasher::operator()(const GlyphDescription& Value) const
	{
		return (static_cast<size_t>(Value.FontID) << 48) | (static_cast<size_t>(Value.FontSize) << 32) | Value.GlyphIndex;
	}
}
