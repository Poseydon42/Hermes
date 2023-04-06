#include "Widget.h"

namespace Hermes::UI
{
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
