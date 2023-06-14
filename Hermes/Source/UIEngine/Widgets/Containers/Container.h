#pragma once

#include <memory>
#include <vector>

#include "Core/Core.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	/*
	 * Abstract class for widget that can contain any other types of widgets.
	 */
	class HERMES_API Container : public Widget
	{
	public:
		virtual ~Container() override = default;

		Container(Container&&) = default;
		Container& operator=(Container&&) = default;

		void AddChild(std::shared_ptr<Widget> Child);

		bool RemoveChild(const Widget* Child);
		void ClearChildren();

		const Widget& GetChild(size_t Index) const;
		Widget& GetChild(size_t Index);

		size_t GetChildrenCount() const;

		virtual void ForEachChild(const ForEachChildConstCallbackType& Callback) const override;
		virtual void ForEachChild(const ForEachChildCallbackType& Callback) override;

	protected:
		Container() = default;

		std::vector<std::shared_ptr<Widget>> Children;
	};
}
