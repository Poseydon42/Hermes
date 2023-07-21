#include "Checkbox.h"

namespace Hermes::UI
{
	std::shared_ptr<Checkbox> Checkbox::Create(std::shared_ptr<Widget> InLabel, bool InInitialState)
	{
		return std::shared_ptr<Checkbox>(new Checkbox(std::move(InLabel), InInitialState));
	}

	bool Checkbox::GetState() const
	{
		return State;
	}

	void Checkbox::SetState(bool NewState)
	{
		State = NewState;
	}

	std::shared_ptr<Widget> Checkbox::GetLabel() const
	{
		return Label;
	}

	void Checkbox::SetLabel(std::shared_ptr<Widget> NewLabel)
	{
		Label = std::move(NewLabel);
	}

	Checkbox::Checkbox(std::shared_ptr<Widget> InLabel, bool InInitialState)
		: State(InInitialState)
		, Label(std::move(InLabel))
	{
	}

	Vec2 Checkbox::ComputePreferredSize() const
	{
		auto LabelSize = Label->ComputePreferredSize();
		LabelSize.X += LabelToBoxDistance + LabelSize.Y;

		return LabelSize;
	}

	void Checkbox::Layout()
	{
		auto LabelBoundingBox = GetBoundingBox();
		LabelBoundingBox.Max.X -= BoundingBox.Height() + LabelToBoxDistance;
		Label->SetBoundingBox(LabelBoundingBox);
		Label->Layout();
	}

	void Checkbox::Draw(DrawingContext& Context) const
	{
		Label->Draw(Context);

		Rect2D OuterRect = BoundingBox;
		OuterRect.Min.X = OuterRect.Max.X - BoundingBox.Height();
		Context.DrawRectangle(OuterRect, Vec4(0.0f), Color, BoxOutlineWidth);

		if (State)
		{
			Rect2D InnerRect = OuterRect;
			InnerRect.Min += BoxOutlineWidth + BoxInternalPadding;
			InnerRect.Max -= BoxOutlineWidth + BoxInternalPadding;
			Context.DrawRectangle(InnerRect, Color);
		}
	}

	bool Checkbox::OnMouseDown(MouseButton Button)
	{
		if (Button != MouseButton::Left)
			return false;

		State = !State;
		return true;
	}

	void Checkbox::ForEachChild(const ForEachChildCallbackType& Callback)
	{
		Callback(*Label);
	}

	void Checkbox::ForEachChild(const ForEachChildConstCallbackType& Callback) const
	{
		Callback(*Label);
	}

	void Checkbox::ForEachChild(const ForEachChildConstSharedPtrCallbackType& Callback) const
	{
		Callback(Label);
	}

	void Checkbox::ForEachChild(const ForEachChildSharedPtrCallbackType& Callback)
	{
		Callback(Label);
	}
}
