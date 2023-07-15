#include "WindowsWindow.h"

#include "ApplicationCore/InputEngine.h"
#include "Core/Profiling.h"
#include "Logging/Logger.h"

#ifdef HERMES_PLATFORM_WINDOWS

namespace Hermes
{
	bool WindowsWindow::ClassRegistered = false;

	static Vec2ui ClientRectToWindowSize(Vec2ui ClientAreaSize, DWORD WindowStyle, DWORD ExStyle)
	{
		Vec2ui Result = ClientAreaSize;
		RECT WindowRect;
		WindowRect.top = 0;
		WindowRect.bottom = static_cast<LONG>(ClientAreaSize.Y);
		WindowRect.left = 0;
		WindowRect.right = static_cast<LONG>(ClientAreaSize.X);
		if (AdjustWindowRectEx(&WindowRect, WindowStyle, false, ExStyle))
		{
			Result.X = WindowRect.right - WindowRect.left;
			Result.Y = WindowRect.bottom - WindowRect.top;
		}
		return Result;
	}
	
	WindowsWindow::WindowsWindow(const String& Name, Vec2ui Size) : IPlatformWindow()
	{
		HINSTANCE AppInstance = GetModuleHandleW(nullptr);
		if (!ClassRegistered)
		{
			WNDCLASSEXW WndClass = {};
			WndClass.cbSize = sizeof(WndClass);
			WndClass.style = CS_OWNDC;
			WndClass.lpfnWndProc = WindowProc;
			WndClass.cbWndExtra = sizeof(this); // We will store pointer to WindowsWindow to properly call member functions on it
			WndClass.hInstance = AppInstance;
			WndClass.lpszClassName = ClassName;

			bool Success = RegisterClassExW(&WndClass);
			HERMES_ASSERT_LOG(Success, "Failed to register window class! Error code: %#010X", GetLastError());
			ClassRegistered = Success;
		}

		CurrentName = Name;
		MessagePump = std::make_unique<EventQueue>();

		static constexpr size_t MaxWindowNameLength = 8192;
		wchar_t UTF16NameBuffer[MaxWindowNameLength];
		MultiByteToWideChar(CP_UTF8, 0, Name.c_str(), -1, UTF16NameBuffer, MaxWindowNameLength);

		Vec2ui WindowSize = ClientRectToWindowSize(Size, WindowStyle, ExStyle);
		WindowHandle = CreateWindowExW(ExStyle, ClassName, UTF16NameBuffer, WindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, static_cast<int>(WindowSize.X), static_cast<int>(WindowSize.Y), nullptr, nullptr, AppInstance, this);

		if (WindowHandle)
		{
			ShowCursor(false);
			UpdateVisibility(true);
		}
		else
		{
			HERMES_LOG_ERROR("Failed to create Win32 window! Error code: %#010X", GetLastError());
		}
	}

	WindowsWindow::~WindowsWindow()
	{
		if (WindowHandle)
			DestroyWindow(WindowHandle);
	}

	void WindowsWindow::UpdateName(const String& NewName)
	{
		static constexpr size_t MaxWindowNameLength = 8192;
		wchar_t Buffer[MaxWindowNameLength];
		MultiByteToWideChar(CP_UTF8, 0, NewName.c_str(), -1, Buffer, MaxWindowNameLength);
		if(SetWindowTextW(WindowHandle, Buffer))
			CurrentName = NewName;
	}

	const String& WindowsWindow::GetName() const
	{
		return CurrentName;
	}

	bool WindowsWindow::ToggleFullscreen(bool Enabled)
	{
		if (Enabled)
		{
			GetWindowPlacement(WindowHandle, &PrevPlacement);
			SetWindowLongW(WindowHandle, GWL_STYLE, WS_POPUP);
			SetWindowLongW(WindowHandle, GWL_EXSTYLE, WS_EX_TOPMOST);
			int DisplayX = GetSystemMetrics(SM_CXSCREEN);
			int DisplayY = GetSystemMetrics(SM_CYSCREEN);
			SetWindowPos(WindowHandle, HWND_TOP, 0, 0, DisplayX, DisplayY, SWP_FRAMECHANGED);
			ShowWindow(WindowHandle, SW_SHOWMAXIMIZED);
		}
		else
		{
			SetWindowLongW(WindowHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			SetWindowLongW(WindowHandle, GWL_EXSTYLE, 0);
			int X = PrevPlacement.rcNormalPosition.left;
			int Y = PrevPlacement.rcNormalPosition.top;
			int W = PrevPlacement.rcNormalPosition.right - X;
			int H = PrevPlacement.rcNormalPosition.bottom - Y;
			SetWindowPos(WindowHandle, HWND_TOP, X, Y, W, H, SWP_FRAMECHANGED);
			ShowWindow(WindowHandle, SW_SHOWDEFAULT);
		}
		return true;
	}

	bool WindowsWindow::UpdateVisibility(bool Visible)
	{
		return ShowWindow(WindowHandle, (Visible ? SW_SHOW : SW_HIDE));
	}

	bool WindowsWindow::Resize(Vec2ui NewSize)
	{
		Vec2ui WindowSize = ClientRectToWindowSize(NewSize, WindowStyle, ExStyle);
		return SetWindowPos(WindowHandle, HWND_NOTOPMOST, 0, 0, static_cast<int>(WindowSize.X), static_cast<int>(WindowSize.Y), SWP_NOMOVE | SWP_NOZORDER);
	}

	Vec2ui WindowsWindow::GetSize() const
	{
		return LastKnownSize;
	}

	bool WindowsWindow::IsValid() const
	{
		return WindowHandle != nullptr;
	}

	EventQueue& WindowsWindow::GetWindowQueue()
	{
		return *MessagePump;
	}

	void WindowsWindow::Run()
	{
		HERMES_PROFILE_FUNC();

		MSG Message;
		while (PeekMessageW(&Message, WindowHandle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		POINT CursorPos;
		if (IsInFocus())
		{
			if (GetCursorPos(&CursorPos) && ScreenToClient(WindowHandle, &CursorPos))
			{
				Vec2i CurrentMousePosition = { CursorPos.x, CursorPos.y };
				auto DeltaMousePosition = CurrentMousePosition - LastCursorPosition;
				LastCursorPosition = CurrentMousePosition;

				MessagePump->PushEvent(WindowMouseMoveEvent(DeltaMousePosition, CurrentMousePosition));

				// NOTE: If cursor is not visible we should move it back to the centre of the screen
				if (!CursorVisibility)
				{
					POINT WindowCenter;
					WindowCenter.x = LastKnownSize.X / 2;
					WindowCenter.y = LastKnownSize.Y / 2;
					LastCursorPosition = { WindowCenter.x, WindowCenter.y };

					ClientToScreen(WindowHandle, &WindowCenter);
					SetCursorPos(WindowCenter.x, WindowCenter.y);
				}
			}
			else
			{
				DWORD ErrorCode = GetLastError();
				HERMES_LOG_WARNING("GetCursorPos() or ScreenToClient() failed, error code: 0x%x", static_cast<uint32>(ErrorCode));
			}
		}

		MessagePump->Run();
	}

	void* WindowsWindow::GetNativeHandle() const
	{
		return WindowHandle;
	}

	void WindowsWindow::SetCursorVisibility(bool IsVisible)
	{
		if (IsVisible != CursorVisibility)
		{
			CursorVisibility = IsVisible;
			ShowCursor(IsVisible);
		}
	}

	Vec2i WindowsWindow::GetCursorPosition() const
	{
		HERMES_ASSERT(CursorVisibility);
		POINT CursorPosition;
		GetCursorPos(&CursorPosition);
		ScreenToClient(WindowHandle, &CursorPosition);
		return { CursorPosition.x, CursorPosition.y };
	}

	bool WindowsWindow::IsInFocus() const
	{
		return (GetFocus() == WindowHandle);
	}

	Vec2i WindowsWindow::GetCursorCoordinatesFromLParam(LPARAM Param)
	{
		auto X = static_cast<int32>(Param & 0xFFFF);
		auto Y = static_cast<int32>(Param >> 16);
		return { X, Y };
	}

	LRESULT WindowsWindow::MessageHandler(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
	{
		switch (Message)
		{
			case WM_DESTROY:
			{
				MessagePump->PushEvent(WindowCloseEvent(GetName()));
				break;
			}
			case WM_SIZE:
			{
				if (WParam == SIZE_MINIMIZED)
				{
					MessagePump->PushEvent(WindowStateEvent(WindowStateEvent::State::Minimized));
					break;
				}
				if (WParam == SIZE_RESTORED)
				{
					WORD NewWidth = LParam & 0xFFFF;
					WORD NewHeight = (LParam >> 16) & 0xFFFF;
					if (NewWidth == LastKnownSize.X && NewHeight == LastKnownSize.Y)
					{
						MessagePump->PushEvent(WindowStateEvent(WindowStateEvent::State::Maximized)); // If size wasn't changed then window was restored from minimized state
					}
					else
					{
						LastKnownSize = { NewWidth, NewHeight };
					}
					break;
				}
				break;
			}
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				static std::unordered_map<uint32, KeyCode> VKCodeToKeyCodeMap =
				{
					{ VK_SPACE, KeyCode::Space },
					{ VK_BACK, KeyCode::Backspace },
					{ VK_TAB, KeyCode::Tab },
					{ VK_RETURN, KeyCode::Enter },
					{ VK_LSHIFT, KeyCode::LeftShift },
					{ VK_RSHIFT, KeyCode::RightShift },
					{ VK_LCONTROL, KeyCode::LeftCtrl },
					{ VK_RCONTROL, KeyCode::RightCtrl },
					{ VK_LMENU, KeyCode::LeftAlt },
					{ VK_RMENU, KeyCode::RightAlt },
					{ VK_PAUSE, KeyCode::Pause },
					{ VK_ESCAPE, KeyCode::Esc },
					{ VK_PRIOR, KeyCode::PageUp },
					{ VK_NEXT, KeyCode::PageDown },
					{ VK_END, KeyCode::End },
					{ VK_HOME, KeyCode::Home },
					{ VK_LEFT, KeyCode::ArrowLeft },
					{ VK_RIGHT, KeyCode::ArrowRight },
					{ VK_UP, KeyCode::ArrowUp },
					{ VK_DOWN, KeyCode::ArrowDown },
					{ VK_SNAPSHOT, KeyCode::PrintScreen },
					{ VK_INSERT, KeyCode::Insert },
					{ VK_DELETE, KeyCode::Delete },
					{ '0', KeyCode::Digit0 },
					{ '1', KeyCode::Digit1 },
					{ '2', KeyCode::Digit2 },
					{ '3', KeyCode::Digit3 },
					{ '4', KeyCode::Digit4 },
					{ '5', KeyCode::Digit5 },
					{ '6', KeyCode::Digit6 },
					{ '7', KeyCode::Digit7 },
					{ '8', KeyCode::Digit8 },
					{ '9', KeyCode::Digit9 },
					{ 'A', KeyCode::A },
					{ 'B', KeyCode::B },
					{ 'C', KeyCode::C },
					{ 'D', KeyCode::D },
					{ 'E', KeyCode::E },
					{ 'F', KeyCode::F },
					{ 'G', KeyCode::G },
					{ 'H', KeyCode::H },
					{ 'I', KeyCode::I },
					{ 'J', KeyCode::J },
					{ 'K', KeyCode::K },
					{ 'L', KeyCode::L },
					{ 'M', KeyCode::M },
					{ 'N', KeyCode::N },
					{ 'O', KeyCode::O },
					{ 'P', KeyCode::P },
					{ 'Q', KeyCode::Q },
					{ 'R', KeyCode::R },
					{ 'S', KeyCode::S },
					{ 'T', KeyCode::T },
					{ 'U', KeyCode::U },
					{ 'V', KeyCode::V },
					{ 'W', KeyCode::W },
					{ 'X', KeyCode::X },
					{ 'Y', KeyCode::Y },
					{ 'Z', KeyCode::Z },
					{ VK_LWIN, KeyCode::LeftWindows },
					{ VK_RWIN, KeyCode::RightWindows },
					{ VK_NUMPAD0, KeyCode::Num0 },
					{ VK_NUMPAD1, KeyCode::Num1 },
					{ VK_NUMPAD2, KeyCode::Num2 },
					{ VK_NUMPAD3, KeyCode::Num3 },
					{ VK_NUMPAD4, KeyCode::Num4 },
					{ VK_NUMPAD5, KeyCode::Num5 },
					{ VK_NUMPAD6, KeyCode::Num6 },
					{ VK_NUMPAD7, KeyCode::Num7 },
					{ VK_NUMPAD8, KeyCode::Num8 },
					{ VK_NUMPAD9, KeyCode::Num9 },
					{ VK_MULTIPLY, KeyCode::Multiply },
					{ VK_ADD, KeyCode::Add },
					{ VK_SUBTRACT, KeyCode::Subtract },
					{ VK_DECIMAL, KeyCode::Decimal },
					{ VK_DIVIDE, KeyCode::Divide },
					{ VK_OEM_PLUS, KeyCode::Add },
					{ VK_OEM_COMMA, KeyCode::Comma },
					{ VK_OEM_MINUS, KeyCode::Subtract },
					{ VK_OEM_PERIOD, KeyCode::Period },
					{ VK_F1, KeyCode::F1 },
					{ VK_F2, KeyCode::F2 },
					{ VK_F3, KeyCode::F3 },
					{ VK_F4, KeyCode::F4 },
					{ VK_F5, KeyCode::F5 },
					{ VK_F6, KeyCode::F6 },
					{ VK_F7, KeyCode::F7 },
					{ VK_F8, KeyCode::F8 },
					{ VK_F9, KeyCode::F9 },
					{ VK_F10, KeyCode::F10 },
					{ VK_F11, KeyCode::F11 },
					{ VK_F12, KeyCode::F12 },
					{ VK_F13, KeyCode::F13 },
					{ VK_F14, KeyCode::F14 },
					{ VK_F15, KeyCode::F15 },
					{ VK_F16, KeyCode::F16 },
					{ VK_F17, KeyCode::F17 },
					{ VK_F18, KeyCode::F18 },
					{ VK_F19, KeyCode::F19 },
					{ VK_F20, KeyCode::F20 },
					{ VK_F21, KeyCode::F21 },
					{ VK_F22, KeyCode::F22 },
					{ VK_F23, KeyCode::F23 },
					{ VK_F24, KeyCode::F24 },
					{ VK_NUMLOCK, KeyCode::NumLock },
					{ VK_SCROLL, KeyCode::ScrollLock },
				};
				bool IsPressEvent = Message == WM_KEYDOWN;
				auto VKCode = static_cast<uint32>(WParam);

				auto Iterator = VKCodeToKeyCodeMap.find(VKCode);
				if (Iterator == VKCodeToKeyCodeMap.end())
				{
					HERMES_LOG_DEBUG("Failed to translate VK code: 0x%02hhx", VKCode);
					break;
				}

				BYTE KeyboardState[256];
				GetKeyboardState(KeyboardState);

				UINT ScanCode = (LParam >> 16) & 0xFF;

				wchar_t Buffer[2];
				auto CharsWritten = ToUnicode(VKCode, ScanCode, KeyboardState, Buffer, 2, 0);

				std::optional<uint32> Codepoint = {};
				if (CharsWritten > 0 && Buffer[1] == 0) // FIXME: this discards any unicode values above 0xFFFF, fix it
				{
					Codepoint = Buffer[0];
				}

				MessagePump->PushEvent(WindowKeyboardEvent(Iterator->second, IsPressEvent, Codepoint));
				break;
			}
			case WM_LBUTTONDOWN:
				MessagePump->PushEvent(WindowMouseButtonEvent(WindowMouseButtonEventType::Pressed, MouseButton::Left, GetCursorCoordinatesFromLParam(LParam)));
				break;
			case WM_LBUTTONUP:
				MessagePump->PushEvent(WindowMouseButtonEvent(WindowMouseButtonEventType::Released, MouseButton::Left, GetCursorCoordinatesFromLParam(LParam)));
				break;
			case WM_RBUTTONDOWN:
				MessagePump->PushEvent(WindowMouseButtonEvent(WindowMouseButtonEventType::Pressed, MouseButton::Right, GetCursorCoordinatesFromLParam(LParam)));
				break;
			case WM_RBUTTONUP:
				MessagePump->PushEvent(WindowMouseButtonEvent(WindowMouseButtonEventType::Released, MouseButton::Right, GetCursorCoordinatesFromLParam(LParam)));
				break;
		default:;
		}

		return DefWindowProcW(Window, Message, WParam, LParam);
	}

	LRESULT WindowsWindow::WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
	{
		if (Message == WM_CREATE)
		{
			SetWindowLongPtrW(Window, 0,
			                  reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCTW*>(LParam)->lpCreateParams));
		}
		auto Instance = reinterpret_cast<WindowsWindow*>(GetWindowLongPtrW(Window, 0));
		if (!Instance) // This means we still haven't got WM_CREATE
			return DefWindowProcW(Window, Message, WParam, LParam);
		return Instance->MessageHandler(Window, Message, WParam, LParam);
	}

	std::unique_ptr<IPlatformWindow> IPlatformWindow::CreatePlatformWindow(const String& Name, Vec2ui Size)
	{
		return std::make_unique<WindowsWindow>(Name, Size);
	}
}

#endif
