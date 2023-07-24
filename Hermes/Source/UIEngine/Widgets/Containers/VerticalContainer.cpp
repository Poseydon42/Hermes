#include "VerticalContainer.h"

#include "Logging/Logger.h"

namespace Hermes::UI
{
	std::shared_ptr<VerticalContainer> VerticalContainer::Create()
	{
		return std::shared_ptr<VerticalContainer>(new VerticalContainer());
	}

	void VerticalContainer::Draw(DrawingContext& Context) const
	{
		for (const auto& Child : Children)
		{
			Child->Draw(Context);
		}
	}

	Vec2 VerticalContainer::ComputePreferredSize() const
	{
		Vec2 Result = {};
		for (const auto& Child : Children)
		{
			auto ChildSize = Child->ComputePreferredSize();
			Result.X = Math::Max(Result.X, ChildSize.X);
			Result.Y += ChildSize.Y;
		}
		return Result;
	}

	void VerticalContainer::Layout()
	{
		/*
		 * Step 1: calculate how much space is required to fit every widget's preferred vertical size
		 * and the total scaling weight of the children that want to be extended.
		 */
		float PreferredVerticalSize = 0.0f;
		float TotalScalingWeight = 0.000000001f; // NOTE: set it to a small value to avoid division by zero
		for (const auto& Child : Children)
		{
			PreferredVerticalSize += Child->ComputePreferredSize().Y;
			if (Child->VerticalScalingPolicy.Type == ScalingType::Extend)
				TotalScalingWeight += Child->VerticalScalingPolicy.ScalingWeight;
		}
		float SizeLeftForExtension = BoundingBox.Height() - PreferredVerticalSize;
		float PixelsPerUnitWeight = SizeLeftForExtension / TotalScalingWeight;

		/*
		 * Step 2: calculate the bounding boxes of the children and call their Layout() function.
		 */
		float NextChildTop = BoundingBox.Top();
		for (const auto& Child : Children)
		{
			auto ChildSize = Child->ComputePreferredSize();
			Rect2D ChildBoundingBox = {};

			float ChildHeight = ChildSize.Y;
			if (Child->VerticalScalingPolicy.Type == ScalingType::Extend)
				ChildHeight += Child->VerticalScalingPolicy.ScalingWeight * PixelsPerUnitWeight;

			ChildBoundingBox.Min.Y = NextChildTop;
			ChildBoundingBox.Max.Y = Math::Min(NextChildTop + ChildHeight, BoundingBox.Bottom());

			float LeftMargin = GetAbsoluteMarginValue(Child->Margins.Left, BoundingBox.Width());
			float RightMargin = GetAbsoluteMarginValue(Child->Margins.Right, BoundingBox.Width());

			float MarginScalingFactor = 1.0f;
			if (LeftMargin + ChildSize.X + RightMargin > BoundingBox.Width())
			{
				float TotalMargin = LeftMargin + RightMargin;
				float AvailableMargin = Math::Max(BoundingBox.Width() - ChildSize.X, 0.0f);
				MarginScalingFactor = AvailableMargin / (TotalMargin + 0.0001f);
			}
			LeftMargin *= MarginScalingFactor;
			RightMargin *= MarginScalingFactor;

			ChildBoundingBox.Min.X = Math::Max(BoundingBox.Min.X + LeftMargin, BoundingBox.Left());
			ChildBoundingBox.Max.X = Math::Min(BoundingBox.Max.X - RightMargin, BoundingBox.Right());

			NextChildTop += ChildBoundingBox.Height();

			Child->SetBoundingBox(ChildBoundingBox);

			Child->Layout();
		}
	}
}
