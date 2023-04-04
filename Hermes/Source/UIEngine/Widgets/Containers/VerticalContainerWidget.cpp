#include "VerticalContainerWidget.h"

namespace Hermes::UI
{
	std::shared_ptr<VerticalContainerWidget> VerticalContainerWidget::Create(std::shared_ptr<Widget> InParent)
	{
		return std::shared_ptr<VerticalContainerWidget>(new VerticalContainerWidget(std::move(InParent)));
	}

	Vec2ui VerticalContainerWidget::GetDimensions() const
	{
		Vec2ui Dimensions = { 0 };

		ForEachChild([&Dimensions](const Widget& Child)
		{
			auto ChildDimensions = Child.GetDimensions();
			Dimensions.X = Math::Max(Dimensions.X, ChildDimensions.X);
			Dimensions.Y += ChildDimensions.Y;
		});

		return Dimensions;
	}

	void VerticalContainerWidget::Draw(DrawingContext& Context, Vec2ui AbsoluteLocation, Vec2ui MaxDimensions) const
	{
		Vec2ui TopLeftCorner = { AbsoluteLocation.X, AbsoluteLocation.Y + MaxDimensions.Y };
		Vec2ui LastChildLocation = TopLeftCorner;
		ForEachChild([&](const Widget& Child)
		{
			auto ChildDimensions = Child.GetDimensions();

			LastChildLocation.Y -= ChildDimensions.Y;
			if (LastChildLocation.Y <= 0)
				return;

			Vec2ui MaxChildDimensions = { MaxDimensions.X, LastChildLocation.Y };
			Child.Draw(Context, LastChildLocation, MaxChildDimensions);
		});
	}

	VerticalContainerWidget::VerticalContainerWidget(std::shared_ptr<Widget> InParent)
		: ContainerWidget(std::move(InParent))
	{
	}
}
