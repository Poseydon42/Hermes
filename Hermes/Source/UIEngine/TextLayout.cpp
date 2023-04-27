#include "TextLayout.h"

#include <freetype/freetype.h>

#include "Core/Profiling.h"
#include "Logging/Logger.h"

namespace Hermes::UI
{
	void TextLayout::Layout(StringView Text, const Font& Font, const TextDrawingCallback& Callback)
	{
		HERMES_PROFILE_FUNC();

		// FIXME: unicode support
		float Cursor = 0.0f;
		float Baseline = 0.0f;
		for (auto Char : Text)
		{
			auto MaybeGlyphIndex = Font.GetGlyphIndex(Char);
			HERMES_ASSERT(MaybeGlyphIndex.has_value());
			auto GlyphIndex = MaybeGlyphIndex.value();

			auto MaybeMetrics = Font.GetGlyphMetrics(GlyphIndex);
			HERMES_ASSERT(MaybeMetrics.has_value());
			auto Metrics = MaybeMetrics.value();

			auto RenderedGlyph = Font.RenderGlyph(GlyphIndex);
			HERMES_ASSERT(RenderedGlyph.has_value());

			float HorizontalPosition = Cursor + Metrics.Bearing.X;
			float VerticalPosition = Baseline + Font.GetMaxAscent() - Metrics.Bearing.Y;

			Callback(GlyphIndex, Vec2(HorizontalPosition, VerticalPosition));

			Cursor += Metrics.Advance;
		}
	}

	Vec2 TextLayout::Measure(StringView Text, const Font& Font)
	{
		HERMES_PROFILE_FUNC();

		// FIXME: optimize!
		float Width = 0.0f;
		Layout(Text, Font, [&](uint32 GlyphIndex, Vec2 GlyphLocation)
		{
			auto GlyphDimensions = Font.GetGlyphDimensions(GlyphIndex);
			Width = GlyphLocation.X + GlyphDimensions.X;
		});
		float Height = Font.GetMaxAscent() + Font.GetMaxDescent();

		return { Width, Height };
	}
}
