#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	class HERMES_API Button : public Widget
	{
	public:
		using CallbackFuncType = std::function<void()>;

		static std::shared_ptr<Button> Create(Vec4 InBackgroundColor);

		void SetLabel(std::shared_ptr<Widget> NewLabel);

		void SetOnPressCallback(CallbackFuncType NewCallback);

	protected:
		std::shared_ptr<Widget> Label;
		Vec4 BackgroundColor;
		CallbackFuncType OnPressCallback;

		explicit Button(Vec4 InBackgroundColor);

		virtual Vec2 ComputePreferredSize() const override;

		virtual void Layout() override;
		
		virtual void Draw(DrawingContext& Context) const override;

		virtual void ForEachChild(const ForEachChildCallbackType& Callback) override;
		virtual void ForEachChild(const ForEachChildConstCallbackType& Callback) const override;

		virtual bool OnMouseDown(MouseButton Button) override;
	};
}
