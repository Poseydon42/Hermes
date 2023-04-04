#include "DrawingContext.h"

#include "RenderingEngine/Renderer.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	void DrawingContext::DrawRectangle(Vec2ui AbsoluteLocation, Vec2ui Dimensions, Vec3 Color)
	{
		Rectangles.emplace_back(AbsoluteLocation, Dimensions, Color);
	}

	const std::vector<DrawingContext::DrawableRectangle>& DrawingContext::GetRectangles() const
	{
		return Rectangles;
	}
}
