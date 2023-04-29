#pragma once

#include <optional>
#include <span>
#include <unordered_map>
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

		float GetMaxAscent(uint32 Size) const;

		float GetMaxDescent(uint32 Size) const;

		std::optional<uint32> GetGlyphIndex(uint32 CharacterCode) const;

		std::optional<GlyphMetrics> GetGlyphMetrics(uint32 GlyphIndex, uint32 Size) const;

		std::optional<RenderedGlyph> RenderGlyph(uint32 GlyphIndex, uint32 Size) const;

	private:
		Font(String InName, std::span<const uint8> BinaryData);

		struct FontData;
		FontData* FontData;

		struct CachedGlyph
		{
			GlyphMetrics Metrics;
			RenderedGlyph Glyph;
		};
		struct GlyphDescription
		{
			uint32 FontSize;
			uint32 GlyphIndex;

			bool operator==(const GlyphDescription& Other) const;
		};
		struct GlyphDescriptionHasher
		{
			size_t operator()(const GlyphDescription& Value) const;
		};
		mutable std::unordered_map<GlyphDescription, CachedGlyph, GlyphDescriptionHasher> CachedGlyphs;

		void SetSize(uint32 Size) const;

		bool LoadGlyph(uint32 GlyphIndex, uint32 FontSize) const;

		void* GetNativeFaceHandle() const;

		friend class TextLayout;
	};
}
