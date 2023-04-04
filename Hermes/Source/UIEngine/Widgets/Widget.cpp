#include "Widget.h"

namespace Hermes::UI
{
	void Widget::SetParent(std::shared_ptr<Widget> NewParent)
	{
		Parent = std::move(NewParent);
	}

	Widget::Widget(std::shared_ptr<Widget> InParent)
		: Parent(std::move(InParent))
	{
	}
}
