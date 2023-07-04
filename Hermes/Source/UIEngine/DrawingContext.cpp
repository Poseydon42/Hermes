#include "DrawingContext.h"

#include "RenderingEngine/Renderer.h"

namespace Hermes::UI
{
	DrawingContext::DrawingContext()
		: WhiteUnitTexture(Texture2D::Create("UI_RECTANGLE_WHITE_TEXTURE", { 1 }, ImageFormat::RGBA, 1, Vec4(1.0f)))
	{
	}

	void DrawingContext::DrawRectangle(Rect2D Rect, Vec4 Color, float CornerRadius)
	{
		Rectangles.push_back(DrawableRectangle {
			.Rect = Rect2Dui(Vec2ui(Rect.Min), Vec2ui(Rect.Max)),
			.Texture = WhiteUnitTexture,
			.Color =Color,
			.TextureWeight = 0.0f,
			.CornerRadius = CornerRadius
		});
	}

	void DrawingContext::DrawRectangle(Rect2D Rect, Vec4 Color, Vec4 OutlineColor, float OutlineRadius, float CornerRadius)
	{
		Rectangles.push_back(DrawableRectangle{
			.Rect = Rect2Dui(Vec2ui(Rect.Min), Vec2ui(Rect.Max)),
			.Texture = WhiteUnitTexture,
			.Color = Color,
			.OutlineColor = OutlineColor,
			.TextureWeight = 0.0f,
			.CornerRadius = CornerRadius,
			.OutlineRadius = OutlineRadius
		});
	}

	void DrawingContext::DrawTexturedRectangle(Rect2D Rect, AssetHandle<Texture2D> Texture)
	{
		Rectangles.push_back(DrawableRectangle{
			.Rect = Rect2Dui(Vec2ui(Rect.Min), Vec2ui(Rect.Max)),
			.Texture = std::move(Texture),
			.TextureWeight = 1.0f,
		});
	}

	void DrawingContext::DrawText(Rect2D Rect, String Text, uint32 TextSize, AssetHandle<Font> Font)
	{
		if (Text.empty())
			return;
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
