#include "PanelWidget.h"

namespace Hermes::UI
{
	std::shared_ptr<PanelWidget> PanelWidget::Create(std::shared_ptr<Widget> InParent, Vec2ui InLocation, Vec2ui InDimensions, Vec3 InColor)
	{
		return std::shared_ptr<PanelWidget>(new PanelWidget(std::move(InParent), InLocation, InDimensions, InColor));
	}

	Vec2ui PanelWidget::GetDimensions() const
	{
		return Dimensions;
	}

	void PanelWidget::Draw(DrawingContext& Context) const
	{
		Widget::Draw(Context);

		Context.DrawRectangle(*this, { 0, 0 }, Dimensions, Color);
	}

	PanelWidget::PanelWidget(std::shared_ptr<Widget> InParent, Vec2ui InLocation, Vec2ui InDimensions, Vec3 InColor)
		: Widget(std::move(InParent), InLocation)
		, Dimensions(InDimensions)
		, Color(InColor)
	{
	}
}
