#include "Widget.h"

namespace Hermes::UI
{
	Vec2ui Widget::GetRelativeLocation() const
	{
		return Location;
	}

	Vec2ui Widget::GetAbsoluteLocation() const
	{
		if (Parent)
			return Parent->GetAbsoluteLocation() + Location;
		return Location;
	}

	Vec2ui Widget::GetDimensions() const
	{
		return { 0 };
	}
	
	void Widget::Draw(DrawingContext&) const
	{
	}

	Widget::Widget(std::shared_ptr<Widget> InParent, Vec2ui InLocation)
		: Location(InLocation)
		, Parent(std::move(InParent))
	{
	}
}
