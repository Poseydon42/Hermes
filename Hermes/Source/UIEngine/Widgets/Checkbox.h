#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	class HERMES_API Checkbox : public Widget
	{
	public:
		static std::shared_ptr<Checkbox> Create(std::shared_ptr<Widget> InLabel, bool InInitialState);

		bool GetState() const;
		void SetState(bool NewState);

		std::shared_ptr<Widget> GetLabel() const;
		void SetLabel(std::shared_ptr<Widget> NewLabel);

	private:
		bool State;
		std::shared_ptr<Widget> Label;

		static constexpr float LabelToBoxDistance = 4.0f;
		static constexpr float BoxOutlineWidth = 2.0f;
		static constexpr float BoxInternalPadding = 2.0f;
		static constexpr Vec4 Color = Vec4(1.0f);

		Checkbox(std::shared_ptr<Widget> InLabel, bool InInitialState);

		virtual Vec2 ComputePreferredSize() const override;

		virtual void Layout() override;;

		virtual void Draw(DrawingContext& Context) const override;

		virtual bool OnMouseDown(MouseButton Button) override;

		virtual void ForEachChild(const ForEachChildCallbackType& Callback) override;
		virtual void ForEachChild(const ForEachChildConstCallbackType& Callback) const override;
		virtual void ForEachChild(const ForEachChildConstSharedPtrCallbackType& Callback) const override;
		virtual void ForEachChild(const ForEachChildSharedPtrCallbackType& Callback) override;
	};
}
