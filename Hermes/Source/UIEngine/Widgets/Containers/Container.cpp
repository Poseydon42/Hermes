#include "Container.h"

namespace Hermes::UI
{
	void Container::AddChild(std::shared_ptr<Widget> Child)
	{
		HERMES_ASSERT(Child);
		Child->SetParent(shared_from_this());
		Children.push_back(std::move(Child));
	}

	bool Container::RemoveChild(const Widget* Child)
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

	void Container::ClearChildren()
	{
		Children.clear();
	}

	const Widget& Container::GetChild(size_t Index) const
	{
		return const_cast<Container*>(this)->GetChild(Index);
	}

	Widget& Container::GetChild(size_t Index)
	{
		HERMES_ASSERT(Index < Children.size());
		return *Children[Index];
	}

	size_t Container::GetChildrenCount() const
	{
		return Children.size();
	}

	void Container::ForEachChild(const ForEachChildConstCallbackType& Callback) const
	{
		for (auto& Child : Children)
			Callback(*Child);
	}

	void Container::ForEachChild(const ForEachChildCallbackType& Callback)
	{
		for (auto& Child : Children)
			Callback(*Child);
	}

	void Container::ForEachChild(const ForEachChildSharedPtrCallbackType& Callback)
	{
		for (auto& Child : Children)
			Callback(Child);
	}

	void Container::ForEachChild(const ForEachChildConstSharedPtrCallbackType& Callback) const
	{
		for (auto& Child : Children)
			Callback(Child);
	}
}
