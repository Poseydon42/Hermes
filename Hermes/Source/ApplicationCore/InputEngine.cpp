#include "InputEngine.h"

#include <optick.h>

#include "ApplicationCore/GameLoop.h"

namespace Hermes
{
	String KeyCodeToString(KeyCode Code)
	{
		switch (Code)
		{
		case KeyCode::LeftMouseButton:
			return L"LeftMouseButton";
		case KeyCode::RightMouseButton:
			return L"RightMouseButton";
		case KeyCode::MiddleMouseButton:
			return L"MiddleMouseButton";
		case KeyCode::FourthMouseButton:
			return L"FourthMouseButton";
		case KeyCode::FifthMouseButton:
			return L"FifthMouseButton";
		case KeyCode::Backspace:
			return L"Backspace";
		case KeyCode::Space:
			return L"Space";
		case KeyCode::Tab:
			return L"Tab";
		case KeyCode::Enter:
			return L"Enter";
		case KeyCode::LeftShift:
			return L"LeftShift";
		case KeyCode::RightShift:
			return L"RightShift";
		case KeyCode::LeftCtrl:
			return L"LeftCtrl";
		case KeyCode::RightCtrl:
			return L"RightCtrl";
		case KeyCode::LeftAlt:
			return L"LeftAlt";
		case KeyCode::RightAlt:
			return L"RightAlt";
		case KeyCode::Pause:
			return L"Pause";
		case KeyCode::Esc:
			return L"Esc";
		case KeyCode::PageUp:
			return L"PageUp";
		case KeyCode::PageDown:
			return L"PageDown";
		case KeyCode::End:
			return L"End";
		case KeyCode::Home:
			return L"Home";
		case KeyCode::ArrowLeft:
			return L"ArrowLeft";
		case KeyCode::ArrowRight:
			return L"ArrowRight";
		case KeyCode::ArrowUp:
			return L"ArrowUp";
		case KeyCode::ArrowDown:
			return L"ArrowDown";
		case KeyCode::PrintScreen:
			return L"PrintScreen";
		case KeyCode::Insert:
			return L"Insert";
		case KeyCode::Delete:
			return L"Delete";
		case KeyCode::Digit0:
			return L"Digit0";
		case KeyCode::Digit1:
			return L"Digit1";
		case KeyCode::Digit2:
			return L"Digit2";
		case KeyCode::Digit3:
			return L"Digit3";
		case KeyCode::Digit4:
			return L"Digit4";
		case KeyCode::Digit5:
			return L"Digit5";
		case KeyCode::Digit6:
			return L"Digit6";
		case KeyCode::Digit7:
			return L"Digit7";
		case KeyCode::Digit8:
			return L"Digit8";
		case KeyCode::Digit9:
			return L"Digit9";
		case KeyCode::A:
			return L"A";
		case KeyCode::B:
			return L"B";
		case KeyCode::C:
			return L"C";
		case KeyCode::D:
			return L"D";
		case KeyCode::E:
			return L"E";
		case KeyCode::F:
			return L"F";
		case KeyCode::G:
			return L"G";
		case KeyCode::H:
			return L"H";
		case KeyCode::I:
			return L"I";
		case KeyCode::J:
			return L"J";
		case KeyCode::K:
			return L"K";
		case KeyCode::L:
			return L"L";
		case KeyCode::M:
			return L"M";
		case KeyCode::N:
			return L"N";
		case KeyCode::O:
			return L"O";
		case KeyCode::P:
			return L"P";
		case KeyCode::Q:
			return L"Q";
		case KeyCode::R:
			return L"R";
		case KeyCode::S:
			return L"S";
		case KeyCode::T:
			return L"T";
		case KeyCode::U:
			return L"U";
		case KeyCode::V:
			return L"V";
		case KeyCode::W:
			return L"W";
		case KeyCode::X:
			return L"X";
		case KeyCode::Y:
			return L"Y";
		case KeyCode::Z:
			return L"Z";
		case KeyCode::LeftWindows:
			return L"LeftWindows";
		case KeyCode::RightWindows:
			return L"RightWindows";
		case KeyCode::Num0:
			return L"Num0";
		case KeyCode::Num1:
			return L"Num1";
		case KeyCode::Num2:
			return L"Num2";
		case KeyCode::Num3:
			return L"Num3";
		case KeyCode::Num4:
			return L"Num4";
		case KeyCode::Num5:
			return L"Num5";
		case KeyCode::Num6:
			return L"Num6";
		case KeyCode::Num7:
			return L"Num7";
		case KeyCode::Num8:
			return L"Num8";
		case KeyCode::Num9:
			return L"Num9";
		case KeyCode::Multiply:
			return L"Multiply";
		case KeyCode::Add:
			return L"Add";
		case KeyCode::Subtract:
			return L"Subtract";
		case KeyCode::Decimal:
			return L"Decimal";
		case KeyCode::Divide:
			return L"Divide";
		case KeyCode::F1:
			return L"F1";
		case KeyCode::F2:
			return L"F2";
		case KeyCode::F3:
			return L"F3";
		case KeyCode::F4:
			return L"F4";
		case KeyCode::F5:
			return L"F5";
		case KeyCode::F6:
			return L"F6";
		case KeyCode::F7:
			return L"F7";
		case KeyCode::F8:
			return L"F8";
		case KeyCode::F9:
			return L"F9";
		case KeyCode::F10:
			return L"F10";
		case KeyCode::F11:
			return L"F11";
		case KeyCode::F12:
			return L"F12";
		case KeyCode::F13:
			return L"F13";
		case KeyCode::F14:
			return L"F14";
		case KeyCode::F15:
			return L"F15";
		case KeyCode::F16:
			return L"F16";
		case KeyCode::F17:
			return L"F17";
		case KeyCode::F18:
			return L"F18";
		case KeyCode::F19:
			return L"F19";
		case KeyCode::F20:
			return L"F20";
		case KeyCode::F21:
			return L"F21";
		case KeyCode::F22:
			return L"F22";
		case KeyCode::F23:
			return L"F23";
		case KeyCode::F24:
			return L"F24";
		case KeyCode::NumLock:
			return L"NumLock";
		case KeyCode::ScrollLock:
			return L"ScrollLock";
		default:
			HERMES_ASSERT(false);
			return L"";
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
		wchar_t Buffer[BufferSize];
		std::swprintf(Buffer, BufferSize, L"KeyEvent: %ls %ls", IsPressEvent() ? L"pressed" : L"released", KeyCodeToString(Code).c_str());
		return String(Buffer);
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
		OPTICK_EVENT();
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
