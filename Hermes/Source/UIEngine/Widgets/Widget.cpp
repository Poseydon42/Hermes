#include "Widget.h"

namespace Hermes::UI
{
	float GetAbsoluteMarginValue(MarginValue Value, float ParentSize)
	{
		if (Value.Type == MarginValueType::Absolute)
			return Value.Value;
		else if (Value.Type == MarginValueType::PercentOfParent)
			return Value.Value * ParentSize;
		else
			HERMES_ASSERT(false);
	}

	float GetRelativeMarginValue(MarginValue Value, float ParentSize)
	{
		if (Value.Type == MarginValueType::Absolute)
			return Value.Value / ParentSize;
		else if (Value.Type == MarginValueType::PercentOfParent)
			return Value.Value;
		else
			HERMES_ASSERT(false);
	}

	const MarginBox& Widget::GetMargins() const
	{
		return Margins;
	}

	MarginBox& Widget::GetMargins()
	{
		return Margins;
	}

	void Widget::SetMargins(MarginBox NewMargins)
	{
		Margins = NewMargins;
	}

	void Widget::SetParent(std::shared_ptr<Widget> NewParent)
	{
		Parent = std::move(NewParent);
	}

	const Widget* Widget::GetParent() const
	{
		return Parent.get();
	}

	Widget* Widget::GetParent()
	{
		return Parent.get();
	}

	Vec2 Widget::ComputeMinimumSize() const
	{
		return {};
	}

	void Widget::Layout()
	{
	}

	void Widget::Draw(DrawingContext&) const
	{
	}

	void Widget::ForEachChild(const ForEachChildCallbackType&)
	{
	}

	Rect2D Widget::GetBoundingBox() const
	{
		return BoundingBox;
	}

	void Widget::SetBoundingBox(Rect2D NewBoundingBox)
	{
		BoundingBox = NewBoundingBox;
	}
}
