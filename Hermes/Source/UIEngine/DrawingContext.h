#pragma once

#include <vector>

#include "AssetSystem/Asset.h"
#include "Core/Core.h"
#include "Math/Rect2D.h"
#include "Math/Vector.h"
#include "UIEngine/Font.h"

namespace Hermes::UI
{
	class Widget;

	/*
	 * An object that is passed to widgets during drawing step so that they can
	 * record primitives that need to be drawn.
	 */
	class HERMES_API DrawingContext
	{
	public:
		struct DrawableRectangle
		{
			/**
			 * Position and size of the rectangle
			 */
			Rect2Dui Rect;

			/**
			 * Color that the rectangle should be filled with
			 */
			Vec4 Color = {};
		};

		/**
		 * Issues a draw of a rectangle
		 *
		 * @param Rect Rectangle to draw
		 * @param Color Color of the rectangle
		 */
		void DrawRectangle(Rect2D Rect, Vec4 Color);

		struct DrawableText
		{
			/*
			 * Position and size of the character on the screen
			 */
			Rect2Dui Rect;

			/*
			 * Text to draw
			 */
			String Text;

			/*
			 * Size of the font, in points
			 */
			uint32 FontSize = 11;

			/*
			 * Character font
			 */
			AssetHandle<Font> Font;
		};

		/*
		 * Draws text in one line
		 */
		void DrawText(Rect2D Rect, String Text, uint32 TextSize, AssetHandle<Font> Font);

		/*
		 * Sets the viewport where the game scene would be rendered.
		 */
		void SetSceneViewport(Rect2D NewViewport);



		const std::vector<DrawableRectangle>& GetRectangles() const;
		
		const std::vector<DrawableText>& GetDrawableTexts() const;

		Rect2Dui GetViewport() const;

	private:
		std::vector<DrawableRectangle> Rectangles;
		std::vector<DrawableText> Texts;

		Rect2Dui Viewport;
	};
}
