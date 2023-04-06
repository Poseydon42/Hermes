#pragma once

#include "Core/Core.h"
#include "Math/Rect2D.h"
#include "Math/Vector2.h"
#include "UIEngine/DrawingContext.h"

namespace Hermes::UI
{
	/*
	 * Base class for any UI element that can be drawn and interacted with.
	 *
	 * Stores a pointer to its parent, however does not itself hold any children.
	 */
	class HERMES_API Widget : public std::enable_shared_from_this<Widget>
	{
	public:
		virtual ~Widget() = default;

		Widget(Widget&&) = default;
		Widget& operator=(Widget&&) = default;

		/*
		 * Sets the new parent for this widget
		 */
		void SetParent(std::shared_ptr<Widget> NewParent);

		/*
		 * Computes the minimum dimensions needed to properly draw this widget
		 * and its children.
		 */
		virtual Vec2 ComputeMinimumDimensions() const = 0;

		/**
		 * Draws this widget and its children within the space that was allocated by its parent.
		 *
		 * @param Context Drawing context used to record drawing primitives
		 * @param AvailableRect Rectangle that the widget can draw in
		 */
		virtual void Draw(DrawingContext& Context, Rect2D AvailableRect) const = 0;

	protected:
		explicit Widget(std::shared_ptr<Widget> InParent);

		std::shared_ptr<Widget> Parent = nullptr;
	};
}
