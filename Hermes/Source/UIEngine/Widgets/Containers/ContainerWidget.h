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
	class HERMES_API ContainerWidget : public Widget
	{
	public:
		virtual ~ContainerWidget() override = default;

		ContainerWidget(ContainerWidget&&) = default;
		ContainerWidget& operator=(ContainerWidget&&) = default;

		void AddChild(std::shared_ptr<Widget> Child);

		bool RemoveChild(const Widget* Child);

		const Widget& GetChild(size_t Index) const;
		Widget& GetChild(size_t Index);

		size_t GetChildrenCount() const;

		virtual void ForEachChild(const ForEachChildConstCallbackType& Callback) const override;
		virtual void ForEachChild(const ForEachChildCallbackType& Callback) override;

	protected:
		ContainerWidget() = default;

		std::vector<std::shared_ptr<Widget>> Children;
	};
}
