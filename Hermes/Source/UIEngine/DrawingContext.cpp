#include "DrawingContext.h"

#include "RenderingEngine/Renderer.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	void DrawingContext::DrawRectangle(const Widget& Widget, Vec2ui RelativeLocation, Vec2ui Dimensions, Vec3 Color)
	{
		auto AbsoluteLocation = Widget.GetAbsoluteLocation() + RelativeLocation;
		Rectangles.emplace_back(AbsoluteLocation, Dimensions, Color);
	}

	const std::vector<DrawingContext::DrawableRectangle>& DrawingContext::GetRectangles() const
	{
		return Rectangles;
	}
}
