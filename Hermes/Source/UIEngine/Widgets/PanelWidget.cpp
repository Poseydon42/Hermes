#include "PanelWidget.h"

namespace Hermes::UI
{
	std::shared_ptr<PanelWidget> PanelWidget::Create(Vec2 InMinimumDimensions, Vec3 InColor)
	{
		return std::shared_ptr<PanelWidget>(new PanelWidget(InMinimumDimensions, InColor));
	}

	void PanelWidget::Draw(DrawingContext& Context, Rect2D AvailableRect) const
	{
		Context.DrawRectangle(AvailableRect, Color);
	}

	PanelWidget::PanelWidget(Vec2 InMinimumDimensions, Vec3 InColor)
		: MinimumDimensions(InMinimumDimensions)
		, Color(InColor)
	{
	}

	Vec2 PanelWidget::ComputeMinimumDimensions() const
	{
		return MinimumDimensions;
	}
}
