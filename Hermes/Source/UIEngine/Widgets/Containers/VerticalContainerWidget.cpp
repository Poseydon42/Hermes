#include "VerticalContainerWidget.h"

#include "Logging/Logger.h"

namespace Hermes::UI
{
	static float GetAbsoluteMarginValue(MarginValue Value, float ParentSize)
	{
		if (Value.Type == MarginValueType::Absolute)
			return Value.Value;
		else if (Value.Type == MarginValueType::PercentOfParent)
			return Value.Value * ParentSize;
		else
			HERMES_ASSERT(false);
	}

	static float GetRelativeMarginValue(MarginValue Value, float ParentSize)
	{
		if (Value.Type == MarginValueType::Absolute)
			return Value.Value / ParentSize;
		else if (Value.Type == MarginValueType::PercentOfParent)
			return Value.Value;
		else
			HERMES_ASSERT(false);
	}

	std::shared_ptr<VerticalContainerWidget> VerticalContainerWidget::Create(std::shared_ptr<Widget> InParent)
	{
		return std::shared_ptr<VerticalContainerWidget>(new VerticalContainerWidget(std::move(InParent)));
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

			auto ChildMinDimensions = Child.ComputeMinimumDimensions();

			auto ChildRect = Rect2D {
				.Min = { AvailableRect.Left(), NextWidgetY + AbsoluteTopMargin },
				.Max = { AvailableRect.Right(), NextWidgetY + AbsoluteTopMargin + ChildMinDimensions.Y }
			};
			ChildRect.Bottom() = Math::Min(ChildRect.Bottom(), AvailableRect.Bottom());

			Child.Draw(Context, ChildRect);

			NextWidgetY += AbsoluteTopMargin + ChildRect.Height() + AbsoluteBottomMargin;
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
