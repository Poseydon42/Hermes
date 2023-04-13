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

	void DrawingContext::SetSceneViewport(Rect2D NewViewport)
	{
		Viewport = {
			Vec2ui(NewViewport.Min),
			Vec2ui(NewViewport.Max)
		};
	}

	const std::vector<DrawingContext::DrawableRectangle>& DrawingContext::GetRectangles() const
	{
		return Rectangles;
	}

	Rect2Dui DrawingContext::GetViewport() const
	{
		return {
			.Min = Vec2ui(Viewport.Min),
			.Max = Vec2ui(Viewport.Max)
		};
	}
}
