#pragma once

#include <functional>

#include "Core/Core.h"
#include "Core/Misc/KeyCode.h"
#include "Math/Rect2D.h"
#include "Math/Vector2.h"
#include "UIEngine/DrawingContext.h"

namespace Hermes::UI
{
	enum class MarginValueType
	{
		Absolute,
		PercentOfParent
	};

	struct MarginValue
	{
		MarginValueType Type = MarginValueType::PercentOfParent;
		float Value = 0.0f;
	};

	/*
	 * Represent margins of the widget in four directions separately.
	 */
	struct MarginBox
	{
		MarginValue Left;
		MarginValue Right;
		MarginValue Top;
		MarginValue Bottom;
	};

	HERMES_API float GetAbsoluteMarginValue(MarginValue Value, float ParentSize);

	HERMES_API float GetRelativeMarginValue(MarginValue Value, float ParentSize);

	/*
	 * Base class for any UI element that can be drawn and interacted with.
	 *
	 * Stores a pointer to its parent, however does not itself hold any children. Stores the margin
	 * box associated with this widget, which is used by its parent to compute its position and size
	 * on the screen. The widget is not responsible for its own layout, it is provided by its parent, the
	 * widget only has to lay out its children in accordance with its layout policy as well as the child's
	 * size and margins.
	 */
	class HERMES_API Widget : public std::enable_shared_from_this<Widget>
	{
	public:
		virtual ~Widget() = default;

		Widget(Widget&&) = default;
		Widget& operator=(Widget&&) = default;

		const MarginBox& GetMargins() const;
		MarginBox& GetMargins();
		void SetMargins(MarginBox NewMargins);

		Rect2D GetBoundingBox() const;
		void SetBoundingBox(Rect2D NewBoundingBox);

		/**
		 * Sets the new parent for this widget
		 */
		void SetParent(std::shared_ptr<Widget> NewParent);

		const Widget* GetParent() const;
		Widget* GetParent();

		/**
		 * Computes the minimum size needed to properly draw this widget and its children.
		 */
		virtual Vec2 ComputeMinimumSize() const;

		/**
		 * Lays out its children within its bounding box.
		 */
		virtual void Layout();

		/**
		 * Draws this widget and its children within its bounding box.
		 *
		 * @param Context Drawing context used to record drawing primitives
		 */
		virtual void Draw(DrawingContext& Context) const;

		/**
		 * Gets called when a mouse button is pressed while the cursor is over the widget.
		 *
		 * @return True if the event was processed in the current widget, false if it has to be passed up the widget hierarchy
		 */
		virtual bool OnMouseDown(MouseButton Button);

		/**
		 * Gets called when a mouse button is released while the cursor is over the widget.
		 *
		 * @return True if the event was processed in the current widget, false if it has to be passed up the widget hierarchy
		 */
		virtual bool OnMouseUp(MouseButton Button);

		using ForEachChildConstCallbackType = std::function<void(const Widget&)>;
		using ForEachChildCallbackType = std::function<void(Widget&)>;

		/**
		 * Calls the provided callback function for each child of current widget, including both
		 * children of container widget as well as parts of composite widgets (e.g. the label of
		 * a button).
		 */
		virtual void ForEachChild(const ForEachChildConstCallbackType& Callback) const;
		virtual void ForEachChild(const ForEachChildCallbackType& Callback);

	protected:
		Widget() = default;

		std::shared_ptr<Widget> Parent = nullptr;
		MarginBox Margins = {};

		Rect2D BoundingBox;
	};
}
