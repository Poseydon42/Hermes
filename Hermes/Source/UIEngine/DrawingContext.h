#pragma once

#include <vector>

#include "AssetSystem/Asset.h"
#include "Core/Core.h"
#include "Math/Rect2D.h"
#include "Math/Vector.h"
#include "RenderingEngine/Texture.h"
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
		DrawingContext();

		struct DrawableRectangle
		{
			/**
			 * Position and size of the rectangle
			 */
			Rect2Dui Rect;

			/**
			 * Texture that will be applied to the rectangle
			 */
			AssetHandle<Texture2D> Texture;

			/**
			 * Color that the rectangle should be filled with
			 */
			Vec4 Color = {};

			/**
			 * Color that the rectangle outline should be filled with
			 */
			Vec4 OutlineColor = {};

			/**
			 * The coefficient by which the sampled texture value will be multiplied. The color value will then be
			 * multiplied by (1.0f - TextureWeight) and the two results will be added to produce the final color of
			 * the rectangle.
			 */
			float TextureWeight = 0.0f;

			/**
			 * Radius of the rectangle corners in pixels, leave as 0 if the rectangle should not have round corners.
			 */
			float CornerRadius = 0.0;

			/**
			 * Radius of the outline of the rectangle in pixels
			 */
			float OutlineRadius = 0.0f;
		};

		/**
		 * Issues a draw of a rectangle
		 *
		 * @param Rect Rectangle to draw
		 * @param Color Color of the rectangle
		 * @param CornerRadius Radius of the rounded corners in pixels, 0 if no rounding should be performed
		 */
		void DrawRectangle(Rect2D Rect, Vec4 Color, float CornerRadius = 0.0f);

		/**
		 * Issues a draw of a rectangle that has an outline with a different color
		 */
		void DrawRectangle(Rect2D Rect, Vec4 Color, Vec4 OutlineColor, float OutlineRadius, float CornerRadius = 0.0f);
		
		/**
		 * Issues a draw of a rectangle that will be filled with the given texture. The texture will be stretched to
		 * fully cover the rectangle, therefore it is the caller's responsibility to ensure that the aspect ration of
		 * the provided rectangle is appropriate.
		 */
		void DrawTexturedRectangle(Rect2D Rect, AssetHandle<Texture2D> Texture);

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

		AssetHandle<Texture2D> WhiteUnitTexture;

		Rect2Dui Viewport;
	};
}
