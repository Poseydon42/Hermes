#pragma once

#include <vector>

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
		 * Returns location of this widget relative to its parent
		 */
		Vec2ui GetRelativeLocation() const;

		/*
		 * Returns location of this widget withing the window that contains it
		 */
		Vec2ui GetAbsoluteLocation() const;

		/*
		 * Returns calculated dimensions of this widget ensuring that all of its children have enough space for themselves
		 */
		virtual Vec2ui GetDimensions() const;

		/*
		 * Draws this widget and its children
		 */
		virtual void Draw(DrawingContext& Context) const;

	protected:
		Widget(std::shared_ptr<Widget> InParent, Vec2ui InLocation);

		Vec2ui Location = { 0 };

		std::shared_ptr<Widget> Parent = nullptr;
	};
}
