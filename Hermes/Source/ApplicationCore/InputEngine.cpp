#include "InputEngine.h"

#include "ApplicationCore/GameLoop.h"

namespace Hermes
{
	String KeyCodeToString(KeyCode Code)
	{
		switch (Code)
		{
		case KeyCode::LeftMouseButton:
			return "LeftMouseButton";
		case KeyCode::RightMouseButton:
			return "RightMouseButton";
		case KeyCode::MiddleMouseButton:
			return "MiddleMouseButton";
		case KeyCode::FourthMouseButton:
			return "FourthMouseButton";
		case KeyCode::FifthMouseButton:
			return "FifthMouseButton";
		case KeyCode::Backspace:
			return "Backspace";
		case KeyCode::Space:
			return "Space";
		case KeyCode::Tab:
			return "Tab";
		case KeyCode::Enter:
			return "Enter";
		case KeyCode::LeftShift:
			return "LeftShift";
		case KeyCode::RightShift:
			return "RightShift";
		case KeyCode::LeftCtrl:
			return "LeftCtrl";
		case KeyCode::RightCtrl:
			return "RightCtrl";
		case KeyCode::LeftAlt:
			return "LeftAlt";
		case KeyCode::RightAlt:
			return "RightAlt";
		case KeyCode::Pause:
			return "Pause";
		case KeyCode::Esc:
			return "Esc";
		case KeyCode::PageUp:
			return "PageUp";
		case KeyCode::PageDown:
			return "PageDown";
		case KeyCode::End:
			return "End";
		case KeyCode::Home:
			return "Home";
		case KeyCode::ArrowLeft:
			return "ArrowLeft";
		case KeyCode::ArrowRight:
			return "ArrowRight";
		case KeyCode::ArrowUp:
			return "ArrowUp";
		case KeyCode::ArrowDown:
			return "ArrowDown";
		case KeyCode::PrintScreen:
			return "PrintScreen";
		case KeyCode::Insert:
			return "Insert";
		case KeyCode::Delete:
			return "Delete";
		case KeyCode::Digit0:
			return "Digit0";
		case KeyCode::Digit1:
			return "Digit1";
		case KeyCode::Digit2:
			return "Digit2";
		case KeyCode::Digit3:
			return "Digit3";
		case KeyCode::Digit4:
			return "Digit4";
		case KeyCode::Digit5:
			return "Digit5";
		case KeyCode::Digit6:
			return "Digit6";
		case KeyCode::Digit7:
			return "Digit7";
		case KeyCode::Digit8:
			return "Digit8";
		case KeyCode::Digit9:
			return "Digit9";
		case KeyCode::A:
			return "A";
		case KeyCode::B:
			return "B";
		case KeyCode::C:
			return "C";
		case KeyCode::D:
			return "D";
		case KeyCode::E:
			return "E";
		case KeyCode::F:
			return "F";
		case KeyCode::G:
			return "G";
		case KeyCode::H:
			return "H";
		case KeyCode::I:
			return "I";
		case KeyCode::J:
			return "J";
		case KeyCode::K:
			return "K";
		case KeyCode::L:
			return "";
		case KeyCode::M:
			return "M";
		case KeyCode::N:
			return "N";
		case KeyCode::O:
			return "O";
		case KeyCode::P:
			return "P";
		case KeyCode::Q:
			return "Q";
		case KeyCode::R:
			return "R";
		case KeyCode::S:
			return "S";
		case KeyCode::T:
			return "T";
		case KeyCode::U:
			return "U";
		case KeyCode::V:
			return "V";
		case KeyCode::W:
			return "W";
		case KeyCode::X:
			return "X";
		case KeyCode::Y:
			return "Y";
		case KeyCode::Z:
			return "Z";
		case KeyCode::LeftWindows:
			return "LeftWindows";
		case KeyCode::RightWindows:
			return "RightWindows";
		case KeyCode::Num0:
			return "Num0";
		case KeyCode::Num1:
			return "Num1";
		case KeyCode::Num2:
			return "Num2";
		case KeyCode::Num3:
			return "Num3";
		case KeyCode::Num4:
			return "Num4";
		case KeyCode::Num5:
			return "Num5";
		case KeyCode::Num6:
			return "Num6";
		case KeyCode::Num7:
			return "Num7";
		case KeyCode::Num8:
			return "Num8";
		case KeyCode::Num9:
			return "Num9";
		case KeyCode::Multiply:
			return "Multiply";
		case KeyCode::Add:
			return "Add";
		case KeyCode::Subtract:
			return "Subtract";
		case KeyCode::Decimal:
			return "Decimal";
		case KeyCode::Divide:
			return "Divide";
		case KeyCode::F1:
			return "F1";
		case KeyCode::F2:
			return "F2";
		case KeyCode::F3:
			return "F3";
		case KeyCode::F4:
			return "F4";
		case KeyCode::F5:
			return "F5";
		case KeyCode::F6:
			return "F6";
		case KeyCode::F7:
			return "F7";
		case KeyCode::F8:
			return "F8";
		case KeyCode::F9:
			return "F9";
		case KeyCode::F10:
			return "F10";
		case KeyCode::F11:
			return "F11";
		case KeyCode::F12:
			return "F12";
		case KeyCode::F13:
			return "F13";
		case KeyCode::F14:
			return "F14";
		case KeyCode::F15:
			return "F15";
		case KeyCode::F16:
			return "F16";
		case KeyCode::F17:
			return "F17";
		case KeyCode::F18:
			return "F18";
		case KeyCode::F19:
			return "F19";
		case KeyCode::F20:
			return "F20";
		case KeyCode::F21:
			return "F21";
		case KeyCode::F22:
			return "F22";
		case KeyCode::F23:
			return "F23";
		case KeyCode::F24:
			return "F24";
		case KeyCode::NumLock:
			return "NumLock";
		case KeyCode::ScrollLock:
			return "ScrollLock";
		default:
			HERMES_ASSERT(false);
			return "";
		}
	}

	KeyEvent::KeyEvent(KeyCode InCode, KeyEventType InEventType)
		: Code(InCode)
		, EventType(InEventType)
	{
	}

	String KeyEvent::ToString() const
	{
		constexpr size_t BufferSize = 128;
		char Buffer[BufferSize];
		std::snprintf(Buffer, BufferSize, "KeyEvent: %s %s", IsPressEvent() ? "pressed" : "released",
		              KeyCodeToString(Code).c_str());
		return { Buffer };
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

	const EventQueue& InputEngine::GetEventQueue() const
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
