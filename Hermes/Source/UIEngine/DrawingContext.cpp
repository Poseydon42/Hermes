#include "DrawingContext.h"

#include "RenderingEngine/Renderer.h"

namespace Hermes::UI
{
	void DrawingContext::DrawRectangle(Rect2D Rect, Vec4 Color)
	{
		// FIXME: proper rounding
		Rectangles.emplace_back(Rect2Dui(Vec2ui(Rect.Min), Vec2ui(Rect.Max)), Color);
	}

	void DrawingContext::DrawText(Rect2D Rect, String Text, uint32 TextSize, AssetHandle<Font> Font)
	{
		Texts.emplace_back(Rect2Dui(Vec2ui(Rect.Min), Vec2ui(Rect.Max)), std::move(Text), TextSize, std::move(Font));
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

	const std::vector<DrawingContext::DrawableText>& DrawingContext::GetDrawableTexts() const
	{
		return Texts;
	}

	Rect2Dui DrawingContext::GetViewport() const
	{
		return {
			.Min = Vec2ui(Viewport.Min),
			.Max = Vec2ui(Viewport.Max)
		};
	}
}
