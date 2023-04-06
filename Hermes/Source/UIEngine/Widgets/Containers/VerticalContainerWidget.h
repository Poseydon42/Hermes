#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/ContainerWidget.h"

namespace Hermes::UI
{
	/*
	 * A container that lays out its children vertically in the order they were added.
	 *
	 * Each child can get up to as much horizontal space as was allocated to the container (taking margins into account),
	 * whereas the vertical space is distributed according to the margins of every element within the container. During
	 * the layout process, minimum dimensions of child widgets have a higher priority than their margins, meaning that
	 * the container will first try to provide every element with its minimum dimensions and only then take margins into
	 * account.
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
