#include "VerticalContainerWidget.h"

#include "Logging/Logger.h"

namespace Hermes::UI
{
	std::shared_ptr<VerticalContainerWidget> VerticalContainerWidget::Create()
	{
		return std::shared_ptr<VerticalContainerWidget>(new VerticalContainerWidget());
	}

	void VerticalContainerWidget::Draw(DrawingContext& Context, Rect2D AvailableRect) const
	{
		float TotalRelativeVerticalMargin = 0.0f;
		float TotalMinimumVerticalSize = 0.0f;
		ForEachChild([&](const Widget& Child)
		{
			TotalRelativeVerticalMargin += GetRelativeMarginValue(Child.GetMargins().Top, AvailableRect.Height());
			TotalRelativeVerticalMargin += GetRelativeMarginValue(Child.GetMargins().Bottom, AvailableRect.Height());

			auto ChildMinimumDimensions = Child.ComputeMinimumDimensions();
			TotalMinimumVerticalSize += ChildMinimumDimensions.Y;
		});
		
		float PixelsLeftForMargins = Math::Max(AvailableRect.Height() - TotalMinimumVerticalSize, 0.0f);
		float TotalVerticalMarginInPixels = AvailableRect.Height() * TotalRelativeVerticalMargin;

		float MarginScalingCoefficient = 1.0f;
		// NOTE: If the children demand more margin (in pixels) than we can provide we need to scale down their vertical margins uniformly
		if (TotalVerticalMarginInPixels > PixelsLeftForMargins)
			MarginScalingCoefficient = PixelsLeftForMargins / TotalVerticalMarginInPixels;

		float NextWidgetY = AvailableRect.Min.Y;
		ForEachChild([&](const Widget& Child)
		{
			float AbsoluteTopMargin = GetAbsoluteMarginValue(Child.GetMargins().Top, AvailableRect.Height()) * MarginScalingCoefficient;
			float AbsoluteBottomMargin = GetAbsoluteMarginValue(Child.GetMargins().Bottom, AvailableRect.Height()) * MarginScalingCoefficient;

			auto AbsoluteLeftMargin = GetAbsoluteMarginValue(Child.GetMargins().Left, AvailableRect.Width());
			auto AbsoluteRightMargin = GetAbsoluteMarginValue(Child.GetMargins().Right, AvailableRect.Width());

			auto ChildMinDimensions = Child.ComputeMinimumDimensions();

			// Again, if the horizontal margins + minimum horizontal size exceed the available width, minimum horizontal size is prioritized
			if (AbsoluteLeftMargin + AbsoluteRightMargin + ChildMinDimensions.X > AvailableRect.Width())
			{
				float ScaleFactor = (AvailableRect.Width() - ChildMinDimensions.X) / (AbsoluteLeftMargin + AbsoluteRightMargin);
				AbsoluteLeftMargin *= ScaleFactor;
				AbsoluteRightMargin *= ScaleFactor;
			}

			auto ChildRect = Rect2D {
				.Min = { AvailableRect.Left() + AbsoluteLeftMargin, NextWidgetY + AbsoluteTopMargin },
				.Max = { AvailableRect.Right() - AbsoluteRightMargin, NextWidgetY + AbsoluteTopMargin + ChildMinDimensions.Y }
			};
			ChildRect.Bottom() = Math::Min(ChildRect.Bottom(), AvailableRect.Bottom());
			HERMES_ASSERT(ChildRect.Right() <= AvailableRect.Right());

			Child.Draw(Context, ChildRect);

			NextWidgetY += AbsoluteTopMargin + ChildRect.Height() + AbsoluteBottomMargin;
		});
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
