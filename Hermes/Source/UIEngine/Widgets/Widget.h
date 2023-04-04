#pragma once

#include "Core/Core.h"
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
		 * Returns calculated dimensions of this widget ensuring that all of its children have enough space for themselves
		 */
		virtual Vec2ui GetDimensions() const = 0;

		/**
		 * Draws this widget and its children
		 *
		 * @param Context Drawing context used to record drawing primitives
		 * @param AbsoluteLocation Absolute location of the bottom left corner of this widget within the window that contains it, in pixels
		 * @param MaxDimensions Maximum dimensions that this widget can use for drawing
		 */
		virtual void Draw(DrawingContext& Context, Vec2ui AbsoluteLocation, Vec2ui MaxDimensions) const = 0;

	protected:
		explicit Widget(std::shared_ptr<Widget> InParent);

		std::shared_ptr<Widget> Parent = nullptr;
	};
}
