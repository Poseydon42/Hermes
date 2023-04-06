#include "PanelWidget.h"

namespace Hermes::UI
{
	std::shared_ptr<PanelWidget> PanelWidget::Create(std::shared_ptr<Widget> InParent, Vec2 InMinimumDimensions, Vec3 InColor)
	{
		return std::shared_ptr<PanelWidget>(new PanelWidget(std::move(InParent), InMinimumDimensions, InColor));
	}

	void PanelWidget::Draw(DrawingContext& Context, Rect2D AvailableRect) const
	{
		auto PreferredRect = Rect2D{
			.Min = { AvailableRect.Left(), AvailableRect.Top() },
			.Max = { AvailableRect.Left() + MinimumDimensions.X, AvailableRect.Top() + MinimumDimensions.Y }
		};

		auto DrawingRect = AvailableRect.Intersect(PreferredRect);
		Context.DrawRectangle(DrawingRect, Color);
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
