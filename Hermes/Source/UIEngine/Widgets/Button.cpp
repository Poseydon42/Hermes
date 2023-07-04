#include "Button.h"

namespace Hermes::UI
{
	std::shared_ptr<Button> Button::Create(Vec4 InBackgroundColor)
	{
		return std::shared_ptr<Button>(new Button(InBackgroundColor));
	}

	void Button::SetLabel(std::shared_ptr<Widget> NewLabel)
	{
		Label = std::move(NewLabel);
		Label->SetParent(shared_from_this());
	}

	void Button::SetOnPressCallback(CallbackFuncType NewCallback)
	{
		OnPressCallback = std::move(NewCallback);
	}

	Button::Button(Vec4 InBackgroundColor)
		: BackgroundColor(InBackgroundColor)
	{
	}

	Vec2 Button::ComputePreferredSize() const
	{
		if (!Label)
			return {};
		return Label->ComputePreferredSize();
	}

	void Button::Layout()
	{
		if (!Label)
			return;
		Label->SetBoundingBox(BoundingBox);
		Label->Layout();
	}

	void Button::Draw(DrawingContext& Context) const
	{
		Context.DrawRectangle(BoundingBox, BackgroundColor);
		if (Label)
			Label->Draw(Context);
	}

	void Button::ForEachChild(const ForEachChildCallbackType& Callback)
	{
		if (Label)
			Callback(*Label);
	}

	void Button::ForEachChild(const ForEachChildConstCallbackType& Callback) const
	{
		if (Label)
			Callback(*Label);
	}

	void Button::ForEachChild(const ForEachChildSharedPtrCallbackType& Callback)
	{
		Callback(Label);
	}

	void Button::ForEachChild(const ForEachChildConstSharedPtrCallbackType& Callback) const
	{
		Callback(Label);
	}

	bool Button::OnMouseDown(MouseButton Button)
	{
		if (Button != MouseButton::Left)
			return false;
		OnPressCallback();
		return true;
	}
}
