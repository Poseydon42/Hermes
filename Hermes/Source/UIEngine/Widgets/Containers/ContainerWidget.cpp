#include "ContainerWidget.h"

namespace Hermes::UI
{
	void ContainerWidget::AddChild(std::shared_ptr<Widget> Child)
	{
		HERMES_ASSERT(Child);
		Child->SetParent(shared_from_this());
		Children.emplace_back(std::move(Child));
	}

	bool ContainerWidget::RemoveChild(const Widget* Child)
	{
		HERMES_ASSERT(Child);

		for (auto Iterator = Children.begin(); Iterator != Children.end(); ++Iterator)
		{
			if (Iterator->get() != Child)
				continue;

			Children.erase(Iterator);
			return true;
		}

		return false;
	}

	const Widget& ContainerWidget::GetChild(size_t Index) const
	{
		return const_cast<ContainerWidget*>(this)->GetChild(Index);
	}

	Widget& ContainerWidget::GetChild(size_t Index)
	{
		HERMES_ASSERT(Index < Children.size());
		return *Children[Index];
	}

	size_t ContainerWidget::GetChildrenCount() const
	{
		return Children.size();
	}

	void ContainerWidget::ForEachChild(const ForEachChildConstCallbackType& Callback) const
	{
		for (auto& Child : Children)
			Callback(*Child);
	}

	void ContainerWidget::ForEachChild(const ForEachChildCallbackType& Callback)
	{
		for (auto& Child : Children)
			Callback(*Child);
	}
}
