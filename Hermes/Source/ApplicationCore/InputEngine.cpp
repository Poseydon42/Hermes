#include "InputEngine.h"

#include "ApplicationCore/GameLoop.h"

namespace Hermes
{
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

	void InputEngine::PushEvent(KeyCode Code, bool WasPressed)
	{
		KeyState[static_cast<int16>(Code)] = WasPressed;
		Queue.PushEvent(KeyEvent(Code, WasPressed ? KeyEventType::Pressed : KeyEventType::Released));
	}

	void InputEngine::ProcessDeferredEvents()
	{
		Queue.Run();
	}

	EventQueue& InputEngine::GetEventQueue()
	{
		return Queue;
	}

	bool InputEngine::IsKeyPressed(KeyCode Key) const
	{
		return KeyState[static_cast<int16>(Key)];
	}

	void InputEngine::SetDeltaMousePosition(Vec2 Position)
	{
		DeltaMousePosition = Position;
	}

	Vec2 InputEngine::GetDeltaMousePosition() const
	{
		return DeltaMousePosition;
	}
}
