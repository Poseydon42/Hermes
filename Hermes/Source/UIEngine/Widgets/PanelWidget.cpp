#include "PanelWidget.h"

namespace Hermes::UI
{
	std::shared_ptr<PanelWidget> PanelWidget::Create(Vec2 InMinimumSize, Vec3 InColor)
	{
		return std::shared_ptr<PanelWidget>(new PanelWidget(InMinimumSize, InColor));
	}

	void PanelWidget::Draw(DrawingContext& Context) const
	{
		Context.DrawRectangle(BoundingBox, Color);
	}

	PanelWidget::PanelWidget(Vec2 InMinimumSize, Vec3 InColor)
		: MinimumSize(InMinimumSize)
		, Color(InColor)
	{
	}

	Vec2 PanelWidget::ComputeMinimumSize() const
	{
		return MinimumSize;
	}
}
