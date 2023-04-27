#pragma once

#include <optional>
#include <span>
#include <vector>

#include "AssetSystem/Asset.h"
#include "Core/Core.h"
#include "Math/Vector2.h"

namespace Hermes::UI
{
	struct GlyphMetrics
	{
		/*
		 * Distance from the origin to top left corner of the glyph's bounding box
		 */
		Vec2 Bearing;

		/*
		 * The amount by which the cursor should be moved after rendering this glyph
		 */
		float Advance = 0.0f;
	};

	struct RenderedGlyph
	{
		/*
		 * Dimensions of the glyph bitmap
		 */
		Vec2ui Dimensions;

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

		Vec2ui GetGlyphDimensions(uint32 GlyphIndex) const;

		float GetMaxAscent() const;

		float GetMaxDescent() const;

		std::optional<uint32> GetGlyphIndex(uint32 CharacterCode) const;

		std::optional<GlyphMetrics> GetGlyphMetrics(uint32 GlyphIndex) const;

		std::optional<RenderedGlyph> RenderGlyph(uint32 GlyphIndex) const;

	private:
		Font(String InName, std::span<const uint8> BinaryData);

		struct FontData;
		FontData* FontData;

		bool LoadGlyph(uint32 GlyphIndex) const;

		void* GetNativeFaceHandle() const;

		friend class TextLayout;
	};
}
