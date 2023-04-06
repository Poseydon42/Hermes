#pragma once

#include <vector>

#include "Core/Core.h"
#include "Math/Rect2D.h"
#include "Math/Vector.h"
#include "Math/Vector2.h"

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
			/*
			 * Position and dimensions of the rectangle
			 */
			Rect2Dui Rect;

			/*
			 * Color that the rectangle should be filled with
			 */
			Vec3 Color = {};
		};

		/**
		 * Issues a draw of a rectangle
		 *
		 * @param Rect Rectangle to draw
		 * @param Color Color of the rectangle
		 */
		void DrawRectangle(Rect2D Rect, Vec3 Color);

		const std::vector<DrawableRectangle>& GetRectangles() const;

	private:
		std::vector<DrawableRectangle> Rectangles;
	};
}
