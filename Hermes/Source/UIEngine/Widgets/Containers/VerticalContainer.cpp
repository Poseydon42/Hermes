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

	Vec2 VerticalContainer::ComputeMinimumSize() const
	{
		Vec2 Result = {};
		for (const auto& Child : Children)
		{
			auto ChildSize = Child->ComputeMinimumSize();
			Result.X = Math::Max(Result.X, ChildSize.X);
			Result.Y += ChildSize.Y;
		}
		return Result;
	}

	void VerticalContainer::Layout()
	{
		float NextChildTop = BoundingBox.Top();
		for (const auto& Child : Children)
		{
			auto ChildSize = Child->ComputeMinimumSize();
			Rect2D ChildBoundingBox = {};

			ChildBoundingBox.Min.Y = NextChildTop;
			ChildBoundingBox.Max.Y = NextChildTop + ChildSize.Y;
			HERMES_ASSERT(ChildBoundingBox.Bottom() <= BoundingBox.Bottom());

			float LeftMargin = GetAbsoluteMarginValue(Child->GetMargins().Left, BoundingBox.Width());
			float RightMargin = GetAbsoluteMarginValue(Child->GetMargins().Right, BoundingBox.Width());

			float MarginScalingFactor = 1.0f;
			if (LeftMargin + ChildSize.X + RightMargin > BoundingBox.Width())
			{
				float TotalMargin = LeftMargin + RightMargin;
				float AvailableMargin = BoundingBox.Width() - ChildSize.X;
				MarginScalingFactor = AvailableMargin / TotalMargin;
			}
			LeftMargin *= MarginScalingFactor;
			RightMargin *= MarginScalingFactor;

			ChildBoundingBox.Min.X = BoundingBox.Min.X + LeftMargin;
			ChildBoundingBox.Max.X = BoundingBox.Max.X - RightMargin;

			NextChildTop += ChildBoundingBox.Height();

			Child->SetBoundingBox(ChildBoundingBox);

			Child->Layout();
		}
	}
}
