#pragma once

#include <optional>

#include "Core/Core.h"
#include "ApplicationCore/Event.h"
#include "ApplicationCore/EventQueue.h"
#include "Math/Vector2.h"

namespace Hermes
{
	enum class KeyCode : int16
	{
		LeftMouseButton = 0,
		RightMouseButton,
		MiddleMouseButton,
		FourthMouseButton,
		FifthMouseButton,
		Space,
		Backspace,
		Tab,
		Enter,
		LeftShift,
		RightShift,
		LeftCtrl,
		RightCtrl,
		LeftAlt,
		RightAlt,
		Pause,
		Esc,
		PageUp,
		PageDown,
		End,
		Home,
		ArrowLeft,
		ArrowRight,
		ArrowUp,
		ArrowDown,
		PrintScreen,
		Insert,
		Delete,
		Digit0,
		Digit1,
		Digit2,
		Digit3,
		Digit4,
		Digit5,
		Digit6,
		Digit7,
		Digit8,
		Digit9,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		LeftWindows,
		RightWindows,
		Num0,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,
		Multiply,
		Add,
		Subtract,
		Decimal,
		Divide,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,
		NumLock,
		ScrollLock,
		Count_
	};

	String KeyCodeToString(KeyCode Code);

	enum class KeyEventType
	{
		Pressed,
		Released
	};

	class HERMES_API KeyEvent : public IEvent
	{
		EVENT_BODY(KeyEvent)

	public:
		KeyEvent(KeyCode InCode, KeyEventType InEventType);

		String ToString() const override;

		KeyCode GetKeyCode() const;
		KeyEventType GetEventType() const;

		bool IsPressEvent() const;
		bool IsReleaseEvent() const;

		std::optional<wchar_t> TryConvertToChar() const;
	private:
		KeyCode Code;
		KeyEventType EventType;
	};

	/*
	 * Holds and processes all pending input messages from platform layer
	 * Currently is a wrapper around EventQueue, but in future will actually implement more functionality
	 * like key-to-action and key-to-axis mapping etc.
	 */
	class HERMES_API InputEngine
	{
	public:
		void PushEvent(KeyCode Code, bool WasPressed);

		void ProcessDeferredEvents();

		const EventQueue& GetEventQueue() const;

		bool IsKeyPressed(KeyCode Key) const;

		void SetDeltaMousePosition(Vec2 Position);
		Vec2 GetDeltaMousePosition() const;
	private:
		EventQueue Queue;
		bool KeyState[static_cast<int16>(KeyCode::Count_)] = {};
		Vec2 DeltaMousePosition;
	};
}
