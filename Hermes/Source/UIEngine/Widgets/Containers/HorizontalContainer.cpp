#include "HorizontalContainer.h"

namespace Hermes::UI
{
	std::shared_ptr<HorizontalContainer> HorizontalContainer::Create()
	{
		return std::shared_ptr<HorizontalContainer>(new HorizontalContainer());
	}

	void HorizontalContainer::Draw(DrawingContext& Context) const
	{
		for (const auto& Child : Children)
		{
			Child->Draw(Context);
		}
	}

	Vec2 HorizontalContainer::ComputePreferredSize() const
	{
		Vec2 Result = {};
		for (const auto& Child : Children)
		{
			auto ChildSize = Child->ComputePreferredSize();
			Result.X += ChildSize.X;
			Result.Y = Math::Max(Result.Y, ChildSize.Y);
		}
		return Result;
	}

	void HorizontalContainer::Layout()
	{
		/*
		 * Step 1: calculate how much space is required to fit every widget's preferred horizontal size
		 * and the total scaling weight of the children that want to be extended.
		 */
		float PreferredHorizontalSize = 0.0f;
		float TotalScalingWeight = 0.000000001f; // NOTE: set it to a small value to avoid division by zero
		for (const auto& Child : Children)
		{
			PreferredHorizontalSize += Child->ComputePreferredSize().X;
			if (Child->HorizontalScalingPolicy.Type == ScalingType::Extend)
				TotalScalingWeight += Child->HorizontalScalingPolicy.ScalingWeight;
		}
		float SizeLeftForExtension = BoundingBox.Width() - PreferredHorizontalSize;
		float PixelsPerUnitWeight = SizeLeftForExtension / TotalScalingWeight;

		/*
		 * Step 2: calculate the bounding boxes of the children and call their Layout() function.
		 */
		float NextChildLeft = BoundingBox.Left();
		for (const auto& Child : Children)
		{
			auto ChildSize = Child->ComputePreferredSize();
			Rect2D ChildBoundingBox = {};

			float ChildWidth = ChildSize.X;
			if (Child->HorizontalScalingPolicy.Type == ScalingType::Extend)
				ChildWidth += Child->HorizontalScalingPolicy.ScalingWeight * PixelsPerUnitWeight;

			ChildBoundingBox.Min.X = NextChildLeft;
			ChildBoundingBox.Max.X = NextChildLeft + ChildWidth;
			HERMES_ASSERT(ChildBoundingBox.Right() <= BoundingBox.Right());

			float TopMargin = GetAbsoluteMarginValue(Child->Margins.Top, BoundingBox.Height());
			float BottomMargin = GetAbsoluteMarginValue(Child->Margins.Bottom, BoundingBox.Height());

			float MarginScalingFactor = 1.0f;
			if (TopMargin + ChildSize.Y + BottomMargin > BoundingBox.Height())
			{
				float TotalMargin = TopMargin + BottomMargin;
				float AvailableMargin = BoundingBox.Height() - ChildSize.Y;
				MarginScalingFactor = AvailableMargin / TotalMargin;
			}
			TopMargin *= MarginScalingFactor;
			BottomMargin *= MarginScalingFactor;

			ChildBoundingBox.Min.Y = BoundingBox.Min.Y + TopMargin;
			ChildBoundingBox.Max.Y = BoundingBox.Max.Y - BottomMargin;

			NextChildLeft += ChildBoundingBox.Width();

			Child->SetBoundingBox(ChildBoundingBox);

			Child->Layout();
		}
	}
}
