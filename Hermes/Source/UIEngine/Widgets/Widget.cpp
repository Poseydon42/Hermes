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

	Widget::Widget(std::shared_ptr<Widget> InParent)
		: Parent(std::move(InParent))
	{
	}
}
