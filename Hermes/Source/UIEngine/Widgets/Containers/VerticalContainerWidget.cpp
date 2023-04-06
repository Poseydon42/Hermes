#include "VerticalContainerWidget.h"

namespace Hermes::UI
{
	std::shared_ptr<VerticalContainerWidget> VerticalContainerWidget::Create(std::shared_ptr<Widget> InParent)
	{
		return std::shared_ptr<VerticalContainerWidget>(new VerticalContainerWidget(std::move(InParent)));
	}

	void VerticalContainerWidget::Draw(DrawingContext& Context, Rect2D AvailableRect) const
	{
		float NextWidgetY = AvailableRect.Min.Y;
		ForEachChild([&](const Widget& Child)
		{
			auto ChildMinDimensions = Child.ComputeMinimumDimensions();

			auto ChildRect = Rect2D(Vec2(AvailableRect.Left(), NextWidgetY), Vec2(AvailableRect.Right(), NextWidgetY + ChildMinDimensions.Y));
			ChildRect.Bottom() = Math::Min(ChildRect.Bottom(), AvailableRect.Bottom());

			NextWidgetY += ChildRect.Height();

			Child.Draw(Context, ChildRect);
		});
	}

	VerticalContainerWidget::VerticalContainerWidget(std::shared_ptr<Widget> InParent)
		: ContainerWidget(std::move(InParent))
	{
	}

	Vec2 VerticalContainerWidget::ComputeMinimumDimensions() const
	{
		Vec2 Result = {};
		ForEachChild([&](const Widget& Child)
		{
			auto ChildMinimumDimensions = Child.ComputeMinimumDimensions();
			Result.X = Math::Max(Result.X, ChildMinimumDimensions.X);
			Result.Y += ChildMinimumDimensions.Y;
		});
		return Result;
	}
}
