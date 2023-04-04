#pragma once

#include <vector>

#include "Core/Core.h"
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
			 * Location, in pixels, relative to the containing window
			 */
			Vec2ui Location = {};

			/*
			 * Dimensions, in pixels
			 */
			Vec2ui Dimensions = {};

			/*
			 * Color that the rectangle should be filled with
			 */
			Vec3 Color = {};
		};

		/**
		 * Issues a draw of a rectangle
		 *
		 * @param AbsoluteLocation Location of the rectangle within the window that contains it, in pixels
		 * @param Dimensions Dimensions of the rectangle
		 * @param Color Color of the rectangle
		 */
		void DrawRectangle(Vec2ui AbsoluteLocation, Vec2ui Dimensions, Vec3 Color);

		const std::vector<DrawableRectangle>& GetRectangles() const;

	private:
		std::vector<DrawableRectangle> Rectangles;
	};
}
