#include "Font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "AssetSystem/AssetLoader.h"
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

	std::optional<FontGlyph> Font::RenderGlyph(uint32 CharacterCode) const
	{
		auto GlyphIndex = FT_Get_Char_Index(FontData->Face, CharacterCode);
		if (!GlyphIndex)
			return std::nullopt;

		auto Error = FT_Load_Glyph(FontData->Face, GlyphIndex, FT_LOAD_DEFAULT);
		if (Error)
		{
			HERMES_LOG_ERROR("Could not load glyph 0x%ux from font %s; FT_Load_Glyph returned error code %i", CharacterCode, GetName().c_str(), Error);
			return std::nullopt;
		}

		Error = FT_Render_Glyph(FontData->Face->glyph, FT_RENDER_MODE_NORMAL);
		if (Error)
		{
			HERMES_LOG_ERROR("Could not render glyph 0x%ux from font %s; FT_Load_Glyph returned error code %i", CharacterCode, GetName().c_str(), Error);
			return std::nullopt;
		}

		static constexpr size_t BytesPerPixel = 1;
		auto Dimensions = Vec2ui(FontData->Face->glyph->bitmap.width, FontData->Face->glyph->bitmap.rows);
		size_t TotalBytes = BytesPerPixel * Dimensions.X * Dimensions.Y;

		auto Bitmap = std::vector(FontData->Face->glyph->bitmap.buffer, FontData->Face->glyph->bitmap.buffer + TotalBytes);

		return FontGlyph{ Dimensions, Bitmap };
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
}
