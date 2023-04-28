#include "TextLayout.h"

#include <freetype/freetype.h>

#include "Core/Profiling.h"

namespace Hermes::UI
{
	void TextLayout::Layout(StringView Text, uint32 FontSize, const Font& Font, const TextDrawingCallback& Callback)
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

			auto MaybeMetrics = Font.GetGlyphMetrics(GlyphIndex, FontSize);
			HERMES_ASSERT(MaybeMetrics.has_value());
			auto Metrics = MaybeMetrics.value();

			float HorizontalPosition = Cursor + Metrics.Bearing.X;
			float VerticalPosition = Baseline + (Font.GetMaxAscent(FontSize) - Metrics.Bearing.Y);

			Callback(GlyphIndex, Vec2(HorizontalPosition, VerticalPosition));

			Cursor += Metrics.Advance;
		}
	}

	Vec2 TextLayout::Measure(StringView Text, uint32 FontSize, const Font& Font)
	{
		HERMES_PROFILE_FUNC();

		// FIXME: optimize!
		float Width = 0.0f;
		Layout(Text, FontSize, Font, [&](uint32 GlyphIndex, Vec2 GlyphLocation)
		{
			auto MaybeGlyphMetrics = Font.GetGlyphMetrics(GlyphIndex, FontSize);
			HERMES_ASSERT(MaybeGlyphMetrics.has_value());
			Width = GlyphLocation.X + MaybeGlyphMetrics.value().Advance;
		});
		float Height = Font.GetMaxAscent(FontSize) + Font.GetMaxDescent(FontSize);

		return { Width, Height };
	}
}
