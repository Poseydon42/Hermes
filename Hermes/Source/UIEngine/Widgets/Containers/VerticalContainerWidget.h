#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/ContainerWidget.h"

namespace Hermes::UI
{
	/*
	 * A container that lays out its children vertically in the order they were added.
	 *
	 * Each child gets as much horizontal space as was allocated to the container, whereas the amount of
	 * vertical space is equal to the minimum vertical size of the child.
	 */
	class HERMES_API VerticalContainerWidget : public ContainerWidget
	{
	public:
		virtual ~VerticalContainerWidget() override = default;

		VerticalContainerWidget(VerticalContainerWidget&&) = default;
		VerticalContainerWidget& operator=(VerticalContainerWidget&&) = default;

		static std::shared_ptr<VerticalContainerWidget> Create(std::shared_ptr<Widget> InParent);

		virtual Vec2 ComputeMinimumDimensions() const override;

		virtual void Draw(DrawingContext& Context, Rect2D AvailableRect) const override;

	protected:
		explicit VerticalContainerWidget(std::shared_ptr<Widget> InParent);
	};
}
