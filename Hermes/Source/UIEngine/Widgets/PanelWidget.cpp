#include "PanelWidget.h"

namespace Hermes::UI
{
	std::shared_ptr<PanelWidget> PanelWidget::Create(std::shared_ptr<Widget> InParent, Vec2ui InDimensions, Vec3 InColor)
	{
		return std::shared_ptr<PanelWidget>(new PanelWidget(std::move(InParent), InDimensions, InColor));
	}

	Vec2ui PanelWidget::GetDimensions() const
	{
		return Dimensions;
	}

	void PanelWidget::Draw(DrawingContext& Context, Vec2ui AbsoluteLocation, Vec2ui MaxDimensions) const
	{
		Vec2ui RectDimensions = { Math::Min(Dimensions.X, MaxDimensions.X), Math::Min(Dimensions.Y, MaxDimensions.Y) };
		Context.DrawRectangle(AbsoluteLocation, RectDimensions, Color);
	}

	PanelWidget::PanelWidget(std::shared_ptr<Widget> InParent, Vec2ui InDimensions, Vec3 InColor)
		: Widget(std::move(InParent))
		, Dimensions(InDimensions)
		, Color(InColor)
	{
	}
}
