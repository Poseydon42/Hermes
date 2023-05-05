#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/Container.h"

namespace Hermes::UI
{
	/*
	 * A container that lays out its children vertically in the order they were added.
	 *
	 * Each child can get up to as much horizontal space as was allocated to the container (taking margins into account).
	 * The amount of vertical space is equal to the child's minimum size in the Y axis if it does not have its vertical
	 * scaling policy set to Extend, otherwise the free vertical space is split between the widgets that have such a scaling
	 * policy according to their scaling weights.
	 *
	 * FIXME: allow children to take as much vertical space as possible.
	 */
	class HERMES_API VerticalContainer : public Container
	{
	public:
		virtual ~VerticalContainer() override = default;

		VerticalContainer(VerticalContainer&&) = default;
		VerticalContainer& operator=(VerticalContainer&&) = default;

		static std::shared_ptr<VerticalContainer> Create();

	protected:
		VerticalContainer() = default;

		virtual Vec2 ComputeMinimumSize() const override;

		virtual void Layout() override;

		virtual void Draw(DrawingContext& Context) const override;
	};
}
