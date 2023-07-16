#include "Widget.h"

namespace Hermes::UI
{
	float GetAbsoluteMarginValue(MarginValue Value, float ParentSize)
	{
		if (Value.Type == MarginValueType::Absolute)
			return Value.Value;
		else if (Value.Type == MarginValueType::PercentOfParent)
			return Value.Value * ParentSize;
		else
			HERMES_ASSERT(false);
	}

	float GetRelativeMarginValue(MarginValue Value, float ParentSize)
	{
		if (Value.Type == MarginValueType::Absolute)
			return Value.Value / ParentSize;
		else if (Value.Type == MarginValueType::PercentOfParent)
			return Value.Value;
		else
			HERMES_ASSERT(false);
	}

	void Widget::SetParent(std::shared_ptr<Widget> NewParent)
	{
		Parent = std::move(NewParent);
	}

	std::shared_ptr<const Widget> Widget::GetParent() const
	{
		return Parent;
	}

	std::shared_ptr<Widget> Widget::GetParent()
	{
		return Parent;
	}

	void Widget::OnUpdate(float)
	{
	}

	Vec2 Widget::ComputePreferredSize() const
	{
		return {};
	}

	void Widget::Layout()
	{
	}

	void Widget::Draw(DrawingContext&) const
	{
	}

	bool Widget::OnMouseDown(MouseButton)
	{
		return false;
	}

	bool Widget::OnMouseUp(MouseButton)
	{
		return false;
	}

	void Widget::OnMouseMove(Vec2, Vec2)
	{
	}

	void Widget::OnKeyDown(KeyCode, std::optional<uint32>)
	{
	}

	void Widget::OnKeyUp(KeyCode, std::optional<uint32>)
	{
	}

	void Widget::ForEachChild(const ForEachChildConstCallbackType&) const
	{
	}

	void Widget::ForEachChild(const ForEachChildCallbackType&)
	{
	}

	void Widget::ForEachChild(const ForEachChildSharedPtrCallbackType&)
	{
	}

	void Widget::ForEachChild(const ForEachChildConstSharedPtrCallbackType&) const
	{
	}

	Rect2D Widget::GetBoundingBox() const
	{
		return BoundingBox;
	}

	void Widget::SetBoundingBox(Rect2D NewBoundingBox)
	{
		BoundingBox = NewBoundingBox;
	}
}
