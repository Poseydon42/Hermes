#include "LabelWidget.h"

#include "UIEngine/TextLayout.h"

namespace Hermes::UI
{
	std::shared_ptr<LabelWidget> LabelWidget::Create(String InText, uint32 InFontSize, AssetHandle<UI::Font> InFont)
	{
		return std::shared_ptr<LabelWidget>(new LabelWidget(std::move(InText), InFontSize, std::move(InFont)));
	}

	Vec2 LabelWidget::ComputeMinimumDimensions() const
	{
		return TextLayout::Measure(Text, FontSize, *Font);
	}

	void LabelWidget::Draw(DrawingContext& Context, Rect2D AvailableRect) const
	{
		auto RequiredDimensions = ComputeMinimumDimensions();
		HERMES_ASSERT(AvailableRect.Width() >= RequiredDimensions.X && AvailableRect.Height() >= RequiredDimensions.Y);

		Rect2D DrawingRect = { AvailableRect.Min, AvailableRect.Min + RequiredDimensions };

		Context.DrawText(DrawingRect, Text, FontSize, Font);
	}

	LabelWidget::LabelWidget(String InText, uint32 InFontSize, AssetHandle<class Font> InFont)
		: Text(std::move(InText))
		, FontSize(InFontSize)
		, Font(std::move(InFont))
	{
	}
}
