#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/Container.h"

namespace Hermes::UI
{
	/*
	 * Represents a viewport that the scene is renderer to. It can also serve as a container for other widgets. Those
	 * widgets will be drawn inside the viewport. There is no strict layout process applied to those widgets - they
	 * are given as much space as the viewport and their margins permit, meaning that some of them might be overlapping.
	 *
	 * There are a lot of things TODO, some of them depend on the renderer as well:
	 *  - allow multiple viewports
	 *	- somehow pass the information about camera to the viewport
	 *	- allow multiple different scenes to be rendered
	 */
	class HERMES_API ViewportContainer : public Container
	{
	public:
		virtual ~ViewportContainer() override = default;

		static std::shared_ptr<ViewportContainer> Create();

	protected:
		ViewportContainer() = default;
		
		virtual Vec2 ComputeMinimumSize() const override;

		virtual void Layout() override;

		virtual void Draw(DrawingContext& Context) const override;
	};
}
