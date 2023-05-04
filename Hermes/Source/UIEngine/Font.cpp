#include "Font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "AssetSystem/AssetLoader.h"
#include "Core/Profiling.h"
#include "Logging/Logger.h"

namespace Hermes::UI
{
	FT_Library GFreeTypeLibrary = nullptr;
	uint32 GNumberOfAliveFonts = 0;

	struct Font::FontData
	{
		std::vector<uint8> FontFileData;
		FT_Face Face = nullptr;
	};

	static constexpr uint32 GFreeTypeSubpixelScalingFactor = 64;

	HERMES_ADD_BINARY_ASSET_LOADER(Font, Font)

	Font::~Font()
	{
		FT_Done_Face(FontData->Face);
		delete FontData;

		if (--GNumberOfAliveFonts == 0)
		{
			FT_Done_FreeType(GFreeTypeLibrary);
			GFreeTypeLibrary = nullptr;
		}
	}

	AssetHandle<Asset> Font::Load(String Name, std::span<const uint8> BinaryData)
	{
		return AssetHandle<Font>(new Font(std::move(Name), BinaryData));
	}

	float Font::GetMaxAscent(uint32 Size) const
	{
		SetSize(Size);
		return static_cast<float>(FontData->Face->size->metrics.ascender) / GFreeTypeSubpixelScalingFactor;
	}

	float Font::GetMaxDescent(uint32 Size) const
	{
		SetSize(Size);
		return static_cast<float>(Math::Abs(FontData->Face->size->metrics.descender)) / GFreeTypeSubpixelScalingFactor;
	}

	std::optional<uint32> Font::GetGlyphIndex(uint32 CharacterCode) const
	{
		auto Index = FT_Get_Char_Index(FontData->Face, CharacterCode);
		if (!Index)
			return std::nullopt;
		return Index;
	}

	std::optional<GlyphMetrics> Font::GetGlyphMetrics(uint32 GlyphIndex, uint32 Size) const
	{
		GlyphDescription Description = { Size, GlyphIndex };
		if (!CachedGlyphs.contains(Description))
		{
			if (!LoadGlyph(GlyphIndex, Size))
				return std::nullopt;
		}
		return CachedGlyphs.at(Description).Metrics;
	}

	std::optional<RenderedGlyph> Font::RenderGlyph(uint32 GlyphIndex, uint32 Size) const
	{
		GlyphDescription Description = { Size, GlyphIndex };
		if (!CachedGlyphs.contains(Description))
		{
			if (!LoadGlyph(GlyphIndex, Size))
				return std::nullopt;
		}
		return CachedGlyphs.at(Description).Glyph;
	}

	Font::Font(String InName, std::span<const uint8> BinaryData)
		: Asset(std::move(InName), AssetType::Font)
	{
		if (!GFreeTypeLibrary)
		{
			HERMES_ASSERT_LOG(FT_Init_FreeType(&GFreeTypeLibrary) == 0, "Failed to initialize FreeType");
		}

		FontData = new struct FontData;
		FontData->FontFileData = std::vector(BinaryData.begin(), BinaryData.end());
		if (auto Error = FT_New_Memory_Face(GFreeTypeLibrary, FontData->FontFileData.data(), static_cast<FT_Long>(FontData->FontFileData.size()), 0, &FontData->Face); Error != 0)
		{
			HERMES_LOG_ERROR("Failed to create FreeType face from font %s; error code %i", GetName().c_str(), Error);
			return;
		}

		GNumberOfAliveFonts++;
	}

	bool Font::GlyphDescription::operator==(const GlyphDescription& Other) const
	{
		return FontSize == Other.FontSize && GlyphIndex == Other.GlyphIndex;
	}

	size_t Font::GlyphDescriptionHasher::operator()(const GlyphDescription& Value) const
	{
		return static_cast<size_t>(Value.FontSize) << 32 | Value.GlyphIndex;
	}

	void Font::SetSize(uint32 Size) const
	{
		// FIXME: this is just the value that Windows uses by default, we should actually query the true DPI and use it here
		static constexpr uint32 ScreenDPI = 96;

		auto ScaledSize = Size * GFreeTypeSubpixelScalingFactor;
		FT_Set_Char_Size(FontData->Face, 0, ScaledSize, ScreenDPI, ScreenDPI);
	}

	bool Font::LoadGlyph(uint32 GlyphIndex, uint32 FontSize) const
	{
		HERMES_PROFILE_FUNC();

		SetSize(FontSize);

		auto Error = FT_Load_Glyph(FontData->Face, GlyphIndex, FT_LOAD_DEFAULT);
		if (Error)
		{
			HERMES_LOG_ERROR("Could not load glyph 0x%ux from font %s; FT_Load_Glyph returned error code %i", GlyphIndex, GetName().c_str(), Error);
			return false;
		}

		Error = FT_Render_Glyph(FontData->Face->glyph, FT_RENDER_MODE_NORMAL);
		if (Error)
		{
			HERMES_LOG_ERROR("Could not render glyph 0x%ux from font %s; FT_Render_Glyph returned error code %i", GlyphIndex, GetName().c_str(), Error);
			return false;
		}

		static constexpr size_t BytesPerPixel = 1;
		auto Dimensions = Vec2ui(FontData->Face->glyph->bitmap.width, FontData->Face->glyph->bitmap.rows);
		size_t TotalBytes = BytesPerPixel * Dimensions.X * Dimensions.Y;
		auto Bitmap = std::vector(FontData->Face->glyph->bitmap.buffer, FontData->Face->glyph->bitmap.buffer + TotalBytes);

		RenderedGlyph RenderedGlyph = {
			.Dimensions = Dimensions,
			.Bitmap = std::move(Bitmap)
		};
		GlyphMetrics Metrics = {
			.Bearing = Vec2(Vec2i(FontData->Face->glyph->metrics.horiBearingX, FontData->Face->glyph->metrics.horiBearingY)) / GFreeTypeSubpixelScalingFactor,
			.Advance = FontData->Face->glyph->advance.x / static_cast<float>(GFreeTypeSubpixelScalingFactor)
		};

		CachedGlyph Glyph = {
			.Metrics = Metrics,
			.Glyph = std::move(RenderedGlyph)
		};
		GlyphDescription Description = { FontSize, GlyphIndex };
		CachedGlyphs[Description] = std::move(Glyph);

		return true;
	}

	void* Font::GetNativeFaceHandle() const
	{
		return FontData->Face;
	}
}
