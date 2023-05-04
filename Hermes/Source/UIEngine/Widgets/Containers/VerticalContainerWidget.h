#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/ContainerWidget.h"

namespace Hermes::UI
{
	/*
	 * A container that lays out its children vertically in the order they were added.
	 *
	 * Each child can get up to as much horizontal space as was allocated to the container (taking margins into account).
	 * The amount of vertical space is equal to the child's minimum size in the Y axis.
	 *
	 * FIXME: allow children to take as much vertical space as possible.
	 */
	class HERMES_API VerticalContainerWidget : public ContainerWidget
	{
	public:
		virtual ~VerticalContainerWidget() override = default;

		VerticalContainerWidget(VerticalContainerWidget&&) = default;
		VerticalContainerWidget& operator=(VerticalContainerWidget&&) = default;

		static std::shared_ptr<VerticalContainerWidget> Create();

	protected:
		VerticalContainerWidget() = default;

		virtual Vec2 ComputeMinimumSize() const override;

		virtual void Layout() override;

		virtual void Draw(DrawingContext& Context) const override;
	};
}
