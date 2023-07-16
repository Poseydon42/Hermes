#include "InputEngine.h"

#include "ApplicationCore/GameLoop.h"

namespace Hermes
{
	EventQueue InputEngine::Queue;
	bool InputEngine::KeyState[static_cast<size_t>(KeyCode::Count_)] = { false };
	bool InputEngine::MouseButtonState[static_cast<size_t>(MouseButton::Count_)] = { false };
	Vec2 InputEngine::CurrentMousePosition = {};
	Vec2 InputEngine::DeltaMousePosition = {};

	static StringView KeyEventTypeToString(KeyEventType Type)
	{
		switch (Type)
		{
		case KeyEventType::Pressed:
			return "pressed";
		case KeyEventType::Released:
			return "released";
		default:
			HERMES_ASSERT(false);
		}
	}

	KeyEvent::KeyEvent(KeyCode InCode, KeyEventType InEventType)
		: Code(InCode)
		, EventType(InEventType)
	{
	}

	String KeyEvent::ToString() const
	{
		return std::format("KeyEvent{{ KeyCode: {}, Type: {} }}", KeyCodeToString(Code), KeyEventTypeToString(EventType));
	}

	KeyCode KeyEvent::GetKeyCode() const
	{
		return Code;
	}

	KeyEventType KeyEvent::GetEventType() const
	{
		return EventType;
	}

	bool KeyEvent::IsPressEvent() const
	{
		return (EventType == KeyEventType::Pressed);
	}

	bool KeyEvent::IsReleaseEvent() const
	{
		return (EventType == KeyEventType::Released);
	}

	std::optional<wchar_t> KeyEvent::TryConvertToChar() const
	{
		// TODO : support for other languages
		switch (Code)
		{
		case KeyCode::A:
			return L'A';
		case KeyCode::B:
			return L'B';
		case KeyCode::C:
			return L'C';
		case KeyCode::D:
			return L'D';
		case KeyCode::E:
			return L'E';
		case KeyCode::F:
			return L'F';
		case KeyCode::G:
			return L'G';
		case KeyCode::H:
			return L'H';
		case KeyCode::I:
			return L'I';
		case KeyCode::J:
			return L'J';
		case KeyCode::K:
			return L'K';
		case KeyCode::L:
			return L'L';
		case KeyCode::M:
			return L'M';
		case KeyCode::N:
			return L'N';
		case KeyCode::O:
			return L'O';
		case KeyCode::P:
			return L'P';
		case KeyCode::Q:
			return L'Q';
		case KeyCode::R:
			return L'R';
		case KeyCode::S:
			return L'S';
		case KeyCode::T:
			return L'T';
		case KeyCode::U:
			return L'U';
		case KeyCode::V:
			return L'V';
		case KeyCode::W:
			return L'W';
		case KeyCode::X:
			return L'X';
		case KeyCode::Y:
			return L'Y';
		case KeyCode::Z:
			return L'Z';
		case KeyCode::Digit0:
		case KeyCode::Num0:
			return L'0';
		case KeyCode::Digit1:
		case KeyCode::Num1:
			return L'1';
		case KeyCode::Digit2:
		case KeyCode::Num2:
			return L'2';
		case KeyCode::Digit3:
		case KeyCode::Num3:
			return L'3';
		case KeyCode::Digit4:
		case KeyCode::Num4:
			return L'4';
		case KeyCode::Digit5:
		case KeyCode::Num5:
			return L'5';
		case KeyCode::Digit6:
		case KeyCode::Num6:
			return L'6';
		case KeyCode::Digit7:
		case KeyCode::Num7:
			return L'7';
		case KeyCode::Digit8:
		case KeyCode::Num8:
			return L'8';
		case KeyCode::Digit9:
		case KeyCode::Num9:
			return L'9';
		case KeyCode::Add:
			return L'+';
		case KeyCode::Subtract:
			return L'-';
		case KeyCode::Multiply:
			return L'*';
		case KeyCode::Divide:
			return L'/';
		case KeyCode::Decimal:
			return L'.';
		case KeyCode::Tab:
			return L'\t';
		case KeyCode::Space:
			return L' ';
		default:
			return {};
		}
	}

	void InputEngine::Init(IPlatformWindow& Window)
	{
		auto& WindowEventQueue = Window.GetWindowQueue();
		WindowEventQueue.Subscribe(WindowKeyboardEvent::GetStaticType(), [](const IEvent& Event) { KeyEventHandler(Event); });
		WindowEventQueue.Subscribe(WindowMouseButtonEvent::GetStaticType(), [](const IEvent& Event) { MouseButtonEventHandler(Event); });
		WindowEventQueue.Subscribe(WindowMouseMoveEvent::GetStaticType(), [](const IEvent& Event) { MouseMoveEventHandler(Event); });
	}

	void InputEngine::ProcessDeferredEvents()
	{
		Queue.Run();
	}

	void InputEngine::Subscribe(IEvent::EventType Type, std::function<EventQueue::CallbackFunctionPrototype> Callback)
	{
		Queue.Subscribe(Type, std::move(Callback));
	}

	bool InputEngine::IsKeyPressed(KeyCode Key)
	{
		return KeyState[static_cast<size_t>(Key)];
	}

	bool InputEngine::IsMouseButtonPressed(MouseButton Button)
	{
		return MouseButtonState[static_cast<size_t>(Button)];
	}

	Vec2 InputEngine::GetCurrentMousePosition()
	{
		return CurrentMousePosition;
	}

	Vec2 InputEngine::GetDeltaMousePosition()
	{
		return DeltaMousePosition;
	}

	void InputEngine::KeyEventHandler(const IEvent& Event)
	{
		HERMES_ASSERT(Event.GetType() == WindowKeyboardEvent::GetStaticType());

		const auto& WindowKeyEvent = static_cast<const WindowKeyboardEvent&>(Event);
		
		KeyState[static_cast<std::underlying_type_t<KeyCode>>(WindowKeyEvent.GetKeyCode())] = WindowKeyEvent.IsPressed();

		KeyEventType EventType;
		if (WindowKeyEvent.IsPressed())
			EventType = KeyEventType::Pressed;
		else if (WindowKeyEvent.IsReleased())
			EventType = KeyEventType::Released;
		else
			HERMES_ASSERT(false);

		Queue.PushEvent(KeyEvent(WindowKeyEvent.GetKeyCode(), EventType));
	}

	void InputEngine::MouseButtonEventHandler(const IEvent& Event)
	{
		HERMES_ASSERT(Event.GetType() == WindowMouseButtonEvent::GetStaticType());

		// FIXME: currently on Windows, this implementation does not receive mouse button release events if
		//        the button was released while cursor was outside of the window rect. This means that the
		//        program mistakenly thinks that the user is still holding mouse button, which might cause us
		//        some problems in the future. Instead, we should directly query the platform layer about whether
		//        a button is pressed.
		const auto& MouseEvent = static_cast<const WindowMouseButtonEvent&>(Event);
		MouseButtonState[static_cast<size_t>(MouseEvent.GetButton())] = (MouseEvent.GetButtonEventType() == WindowMouseButtonEventType::Pressed);
	}

	void InputEngine::MouseMoveEventHandler(const IEvent& Event)
	{
		HERMES_ASSERT(Event.GetType() == WindowMouseMoveEvent::GetStaticType());

		const auto& MouseEvent = static_cast<const WindowMouseMoveEvent&>(Event);

		CurrentMousePosition = Vec2(MouseEvent.GetCursorCoordinates());
		DeltaMousePosition = Vec2(MouseEvent.GetMouseDelta());
	}
}
