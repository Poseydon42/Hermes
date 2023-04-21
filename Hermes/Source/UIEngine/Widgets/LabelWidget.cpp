#include "LabelWidget.h"

namespace Hermes::UI
{
	std::shared_ptr<LabelWidget> LabelWidget::Create(std::shared_ptr<Widget> InParent, String InText, AssetHandle<UI::Font> InFont)
	{
		return std::shared_ptr<LabelWidget>(new LabelWidget(std::move(InParent), std::move(InText), std::move(InFont)));
	}

	Vec2 LabelWidget::ComputeMinimumDimensions() const
	{
		if (Text.empty())
			return {};
		return Vec2(Font->GetGlyphDimensions(Text[0]));
	}

	void LabelWidget::Draw(DrawingContext& Context, Rect2D AvailableRect) const
	{
		auto RequiredDimensions = ComputeMinimumDimensions();
		HERMES_ASSERT(AvailableRect.Width() >= RequiredDimensions.X && AvailableRect.Height() >= RequiredDimensions.Y);

		Rect2D DrawingRect = { AvailableRect.Min, AvailableRect.Min + RequiredDimensions };

		Context.DrawText(DrawingRect, Text, Font);
	}

	LabelWidget::LabelWidget(std::shared_ptr<Widget> InParent, String InText, AssetHandle<class Font> InFont)
		: Widget(std::move(InParent))
		, Text(std::move(InText))
		, Font(std::move(InFont))
	{
	}
}
