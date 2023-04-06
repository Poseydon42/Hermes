#include "PanelWidget.h"

namespace Hermes::UI
{
	std::shared_ptr<PanelWidget> PanelWidget::Create(std::shared_ptr<Widget> InParent, Vec2 InMinimumDimensions, Vec3 InColor)
	{
		return std::shared_ptr<PanelWidget>(new PanelWidget(std::move(InParent), InMinimumDimensions, InColor));
	}

	void PanelWidget::Draw(DrawingContext& Context, Rect2D AvailableRect) const
	{
		Context.DrawRectangle(AvailableRect, Color);
	}

	PanelWidget::PanelWidget(std::shared_ptr<Widget> InParent, Vec2 InMinimumDimensions, Vec3 InColor)
		: Widget(std::move(InParent))
		, MinimumDimensions(InMinimumDimensions)
		, Color(InColor)
	{
	}

	Vec2 PanelWidget::ComputeMinimumDimensions() const
	{
		return MinimumDimensions;
	}
}
