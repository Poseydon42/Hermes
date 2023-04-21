#pragma once

#include <optional>
#include <span>
#include <vector>

#include "AssetSystem/Asset.h"
#include "Core/Core.h"
#include "Math/Vector2.h"

namespace Hermes::UI
{
	struct FontGlyph
	{
		/*
		 * Dimensions of the glyph bitmap
		 */
		Vec2ui Dimensions;

		/*
		 * Distance from the cursor position to the top left corner of the bitmap
		 */
		Vec2 Bearing;

		/*
		 * Horizontal distance from the cursor position to the next cursor position
		 */
		float Advance;

		/*
		 * Glyph bitmap that contains a single channel that should be treated as the alpha channel
		 */
		std::vector<uint8> Bitmap;
	};

	class HERMES_API Font : public Asset
	{
		HERMES_DECLARE_ASSET(Font)

	public:
		virtual ~Font() override;

		static AssetHandle<Asset> Load(String Name, std::span<const uint8> BinaryData);

		Vec2ui GetGlyphDimensions(uint32 CharacterCode) const;

		std::optional<FontGlyph> RenderGlyph(uint32 CharacterCode) const;

	private:
		Font(String InName, std::span<const uint8> BinaryData);

		struct FontData;
		FontData* FontData;
	};
}
