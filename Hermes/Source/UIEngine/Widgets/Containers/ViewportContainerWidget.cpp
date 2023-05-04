#include "ViewportContainerWidget.h"

namespace Hermes::UI
{
	std::shared_ptr<ViewportContainerWidget> ViewportContainerWidget::Create()
	{
		return std::shared_ptr<ViewportContainerWidget>(new ViewportContainerWidget());
	}

	Vec2 ViewportContainerWidget::ComputeMinimumSize() const
	{
		Vec2 MinSize = {};
		for (const auto& Child : Children)
		{
			auto ChildMinSize = Child->ComputeMinimumSize();
			MinSize.X = Math::Max(MinSize.X, ChildMinSize.X);
			MinSize.Y = Math::Max(MinSize.Y, ChildMinSize.Y);
		}
		return MinSize;
	}

	void ViewportContainerWidget::Layout()
	{
		for (const auto& Child : Children)
		{
			auto AbsoluteMarginLeft   = GetAbsoluteMarginValue(Child->GetMargins().Left,   BoundingBox.Width());
			auto AbsoluteMarginRight  = GetAbsoluteMarginValue(Child->GetMargins().Right,  BoundingBox.Width());
			auto AbsoluteMarginTop    = GetAbsoluteMarginValue(Child->GetMargins().Top,    BoundingBox.Height());
			auto AbsoluteMarginBottom = GetAbsoluteMarginValue(Child->GetMargins().Bottom, BoundingBox.Height());

			auto ChildMinSize = Child->ComputeMinimumSize();

			float TotalUnscaledWidth = AbsoluteMarginLeft + ChildMinSize.X + AbsoluteMarginRight;
			if (TotalUnscaledWidth > BoundingBox.Width())
			{
				float ScalingFactor = (BoundingBox.Width() - ChildMinSize.X) / (TotalUnscaledWidth - ChildMinSize.X);
				AbsoluteMarginLeft *= ScalingFactor;
				AbsoluteMarginRight *= ScalingFactor;
			}

			float TotalUnscaledHeight = AbsoluteMarginTop + ChildMinSize.Y + AbsoluteMarginBottom;
			if (TotalUnscaledHeight > BoundingBox.Height())
			{
				float ScalingFactor = (BoundingBox.Height() - ChildMinSize.Y) / (TotalUnscaledHeight - ChildMinSize.Y);
				AbsoluteMarginTop *= ScalingFactor;
				AbsoluteMarginBottom *= ScalingFactor;
			}

			Rect2D ChildBoundingBox = {
				.Min = { BoundingBox.Left() + AbsoluteMarginLeft, BoundingBox.Top() + AbsoluteMarginTop },
				.Max = { BoundingBox.Right() - AbsoluteMarginRight, BoundingBox.Bottom() - AbsoluteMarginBottom }
			};
			HERMES_ASSERT(ChildBoundingBox.Width() >= ChildMinSize.X);
			HERMES_ASSERT(ChildBoundingBox.Height() >= ChildMinSize.Y);

			Child->SetBoundingBox(ChildBoundingBox);

			Child->Layout();
		}
	}

	void ViewportContainerWidget::Draw(DrawingContext& Context) const
	{
		Context.SetSceneViewport(BoundingBox);
		for (const auto& Child : Children)
		{
			Child->Draw(Context);
		}
	}
}
