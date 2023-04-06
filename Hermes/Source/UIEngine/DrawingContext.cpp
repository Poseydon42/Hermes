#include "DrawingContext.h"

#include "RenderingEngine/Renderer.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	void DrawingContext::DrawRectangle(Rect2D Rect, Vec3 Color)
	{
		// FIXME: proper rounding
		Rectangles.emplace_back(Rect2Dui(Vec2ui(Rect.Min), Vec2ui(Rect.Max)), Color);
	}

	const std::vector<DrawingContext::DrawableRectangle>& DrawingContext::GetRectangles() const
	{
		return Rectangles;
	}
}
