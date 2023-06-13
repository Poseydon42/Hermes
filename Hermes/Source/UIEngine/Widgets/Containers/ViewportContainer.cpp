#include "ViewportContainer.h"

namespace Hermes::UI
{
	std::shared_ptr<ViewportContainer> ViewportContainer::Create()
	{
		return std::shared_ptr<ViewportContainer>(new ViewportContainer());
	}

	Vec2 ViewportContainer::ComputePreferredSize() const
	{
		Vec2 PreferredSize = {};
		for (const auto& Child : Children)
		{
			auto ChildPreferredSize = Child->ComputePreferredSize();
			PreferredSize.X = Math::Max(PreferredSize.X, ChildPreferredSize.X);
			PreferredSize.Y = Math::Max(PreferredSize.Y, ChildPreferredSize.Y);
		}
		return PreferredSize;
	}

	void ViewportContainer::Layout()
	{
		for (const auto& Child : Children)
		{
			auto AbsoluteMarginLeft   = GetAbsoluteMarginValue(Child->Margins.Left,   BoundingBox.Width());
			auto AbsoluteMarginRight  = GetAbsoluteMarginValue(Child->Margins.Right,  BoundingBox.Width());
			auto AbsoluteMarginTop    = GetAbsoluteMarginValue(Child->Margins.Top,    BoundingBox.Height());
			auto AbsoluteMarginBottom = GetAbsoluteMarginValue(Child->Margins.Bottom, BoundingBox.Height());

			auto ChildPreferredSize = Child->ComputePreferredSize();

			float TotalUnscaledWidth = AbsoluteMarginLeft + ChildPreferredSize.X + AbsoluteMarginRight;
			if (TotalUnscaledWidth > BoundingBox.Width())
			{
				float ScalingFactor = (BoundingBox.Width() - ChildPreferredSize.X) / (TotalUnscaledWidth - ChildPreferredSize.X);
				AbsoluteMarginLeft *= ScalingFactor;
				AbsoluteMarginRight *= ScalingFactor;
			}

			float TotalUnscaledHeight = AbsoluteMarginTop + ChildPreferredSize.Y + AbsoluteMarginBottom;
			if (TotalUnscaledHeight > BoundingBox.Height())
			{
				float ScalingFactor = (BoundingBox.Height() - ChildPreferredSize.Y) / (TotalUnscaledHeight - ChildPreferredSize.Y);
				AbsoluteMarginTop *= ScalingFactor;
				AbsoluteMarginBottom *= ScalingFactor;
			}

			Rect2D ChildBoundingBox = {
				.Min = { BoundingBox.Left() + AbsoluteMarginLeft, BoundingBox.Top() + AbsoluteMarginTop },
				.Max = { BoundingBox.Right() - AbsoluteMarginRight, BoundingBox.Bottom() - AbsoluteMarginBottom }
			};
			HERMES_ASSERT(ChildBoundingBox.Width() >= ChildPreferredSize.X);
			HERMES_ASSERT(ChildBoundingBox.Height() >= ChildPreferredSize.Y);

			Child->SetBoundingBox(ChildBoundingBox);

			Child->Layout();
		}
	}

	void ViewportContainer::Draw(DrawingContext& Context) const
	{
		Context.SetSceneViewport(BoundingBox);
		for (const auto& Child : Children)
		{
			Child->Draw(Context);
		}
	}
}
