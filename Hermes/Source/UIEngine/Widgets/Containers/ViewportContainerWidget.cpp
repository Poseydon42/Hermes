#include "ViewportContainerWidget.h"

namespace Hermes::UI
{
	std::shared_ptr<ViewportContainerWidget> ViewportContainerWidget::Create()
	{
		return std::shared_ptr<ViewportContainerWidget>(new ViewportContainerWidget());
	}

	Vec2 ViewportContainerWidget::ComputeMinimumDimensions() const
	{
		Vec2 MinDimensions = {};
		ForEachChild([&](const Widget& Child)
		{
			auto ChildMinDimensions = Child.ComputeMinimumDimensions();
			MinDimensions.X = Math::Max(MinDimensions.X, ChildMinDimensions.X);
			MinDimensions.Y = Math::Max(MinDimensions.Y, ChildMinDimensions.Y);
		});
		return MinDimensions;
	}

	void ViewportContainerWidget::Draw(DrawingContext& Context, Rect2D AvailableRect) const
	{
		Context.SetSceneViewport(AvailableRect);

		ForEachChild([&](const Widget& Child)
		{
			auto AbsoluteMarginLeft = GetAbsoluteMarginValue(Child.GetMargins().Left, AvailableRect.Width());
			auto AbsoluteMarginRight = GetAbsoluteMarginValue(Child.GetMargins().Right, AvailableRect.Width());
			auto AbsoluteMarginTop = GetAbsoluteMarginValue(Child.GetMargins().Top, AvailableRect.Height());
			auto AbsoluteMarginBottom = GetAbsoluteMarginValue(Child.GetMargins().Bottom, AvailableRect.Height());

			auto ChildMinDimensions = Child.ComputeMinimumDimensions();

			float TotalUnscaledWidth = AbsoluteMarginLeft + ChildMinDimensions.X + AbsoluteMarginRight;
			if (TotalUnscaledWidth > AvailableRect.Width())
			{
				float ScalingFactor = (AvailableRect.Width() - ChildMinDimensions.X) / (TotalUnscaledWidth - ChildMinDimensions.X);
				AbsoluteMarginLeft *= ScalingFactor;
				AbsoluteMarginRight *= ScalingFactor;
			}

			float TotalUnscaledHeight = AbsoluteMarginTop + ChildMinDimensions.Y + AbsoluteMarginBottom;
			if (TotalUnscaledHeight > AvailableRect.Height())
			{
				float ScalingFactor = (AvailableRect.Height() - ChildMinDimensions.Y) / (TotalUnscaledHeight - ChildMinDimensions.Y);
				AbsoluteMarginTop *= ScalingFactor;
				AbsoluteMarginBottom *= ScalingFactor;
			}

			Rect2D ChildRect = {
				.Min = { AbsoluteMarginLeft, AbsoluteMarginTop },
				.Max = { AvailableRect.Width() - AbsoluteMarginRight, AvailableRect.Height() - AbsoluteMarginBottom }
			};
			HERMES_ASSERT(ChildRect.Width() >= ChildMinDimensions.X);
			HERMES_ASSERT(ChildRect.Height() >= ChildMinDimensions.Y);

			Child.Draw(Context, ChildRect);
		});
	}
}
