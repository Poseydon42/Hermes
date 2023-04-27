#pragma once

#include <functional>

#include "Core/Core.h"
#include "UIEngine/Font.h"

namespace Hermes::UI
{
	class HERMES_API TextLayout
	{
	public:
		/*
		 * Callback function that the layout system calls to draw a character to the screen at some location.
		 *
		 * The first argument is the character code to draw, the second argument is the position of the top left corner of the
		 * rendered glyph relative to the left edge of the text bounding box in X direction and relative to the text baseline in Y direction.
		 */
		using TextDrawingCallback = std::function<void(uint32, Vec2)>;

		/*
		 * Performs the layout of the provided text using the provided font. Calls callback for every glyph
		 * that needs to be drawn to the screen.
		 */
		static void Layout(StringView Text, const Font& Font, const TextDrawingCallback& Callback);

		/*
		 * Returns the number of pixels in X and Y direction required to draw the provided line of text
		 * with the provided font.
		 */
		static Vec2 Measure(StringView Text, const Font& Font);
	};
}
