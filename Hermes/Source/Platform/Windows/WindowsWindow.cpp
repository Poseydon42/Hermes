#include "WindowsWindow.h"

#include "Core/Application/GameLoop.h"

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
		HINSTANCE AppInstance = GetModuleHandleW(NULL);
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
			HERMES_ASSERT_LOG(Success, L"Failed to register window class! Error code: %#010X", GetLastError());
			ClassRegistered = Success;
		}

		CurrentName = Name;
		MessagePump = std::make_shared<EventQueue>();

		
		Vec2ui WindowSize = ClientRectToWindowSize(Size, WindowStyle, ExStyle);
		WindowHandle = CreateWindowExW(
			ExStyle, ClassName, Name.c_str(), WindowStyle,
			CW_USEDEFAULT, CW_USEDEFAULT, WindowSize.X, WindowSize.Y,
			0, 0, AppInstance, this);
		if (WindowHandle)
		{
			UpdateVisibility(true);
		}
		else
		{
			HERMES_LOG_ERROR(L"Failed to create Win32 window! Error code: %#010X", GetLastError());
		}
	}

	WindowsWindow::~WindowsWindow()
	{
		if (WindowHandle)
			DestroyWindow(WindowHandle);
	}

	WindowsWindow::WindowsWindow(WindowsWindow&& Other) : IPlatformWindow(std::move(Other))
	{
		*this = std::move(Other);
	}

	WindowsWindow& WindowsWindow::operator=(WindowsWindow&& Other)
	{
		std::swap(PrevPlacement, Other.PrevPlacement);
		std::swap(MessagePump, Other.MessagePump);
		std::swap(WindowHandle, Other.WindowHandle);
		std::swap(CurrentName, Other.CurrentName);
		std::swap(LastKnownSize, Other.LastKnownSize);
		return *this;
	}

	void WindowsWindow::UpdateName(const String& NewName)
	{
		if(SetWindowTextW(WindowHandle, NewName.c_str()))
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
		return SetWindowPos(WindowHandle, HWND_NOTOPMOST, 0, 0, WindowSize.X, WindowSize.Y, SWP_NOMOVE | SWP_NOZORDER);
	}

	Vec2ui WindowsWindow::GetSize() const
	{
		return LastKnownSize;
	}

	bool WindowsWindow::IsValid() const
	{
		return WindowHandle != 0;
	}

	std::weak_ptr<EventQueue> WindowsWindow::WindowQueue() const
	{
		return MessagePump;
	}

	void WindowsWindow::Run() const
	{
		MSG Message;

		while (PeekMessageW(&Message, WindowHandle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		MessagePump->Run();
	}

	void* WindowsWindow::GetNativeHandle() const
	{
		return (void*)WindowHandle;
	}

	LRESULT WindowsWindow::MessageHandler(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
	{
		switch (Message)
		{
		case WM_DESTROY:
			MessagePump->PushEvent(WindowCloseEvent(GetName()));
			break;
		case WM_SIZE:
		{
			if (WParam == SIZE_MINIMIZED)
			{
				GGameLoop->SetPause(true);
				break;
			}
			if (WParam == SIZE_RESTORED)
			{
				WORD NewWidth = LParam & 0xFFFF;
				WORD NewHeight = (LParam >> 16) & 0xFFFF;
				if (NewWidth == LastKnownSize.X && NewHeight == LastKnownSize.Y)
				{
					GGameLoop->SetPause(false); // If size wasn't changed then window was restored from minimized state
				}
				else
				{
					LastKnownSize = { NewWidth, NewHeight };
				}
				break;
			}
		}
		// WM_KEYDOWN and WM_PAINT handling is temporary to check whether fullscreen works properly
		// TODO : remove this debug stuff when it's no longer needed
		case WM_KEYDOWN:
			if (WParam == VK_LEFT)
				ToggleFullscreen(true);
			if (WParam == VK_RIGHT)
				ToggleFullscreen(false);
			break;
		case WM_PAINT:
			HERMES_LOG_INFO(L"Got WM_PAINT message.");
			PAINTSTRUCT Paint;
			RECT ClientRect;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			GetClientRect(Window, &ClientRect);
			HBRUSH TempBrush = CreateSolidBrush(RGB(255, 255, 255));
			FillRect(DeviceContext, &ClientRect, TempBrush);
			DeleteObject(TempBrush);
			EndPaint(Window, &Paint);
		}
		return DefWindowProcW(Window, Message, WParam, LParam);
	}

	LRESULT WindowsWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_CREATE)
		{
			SetWindowLongPtrW(hwnd, 0, (LONG_PTR)((CREATESTRUCTW*)(lParam))->lpCreateParams);
		}
		auto Instance = (WindowsWindow*)GetWindowLongPtrW(hwnd, 0);
		if (!Instance) // This means we still haven't got WM_CREATE
			return DefWindowProcW(hwnd, uMsg, wParam, lParam);
		return Instance->MessageHandler(hwnd, uMsg, wParam, lParam);
	}

	std::shared_ptr<IPlatformWindow> IPlatformWindow::CreatePlatformWindow(const String& Name, Vec2ui Size)
	{
		return std::make_shared<WindowsWindow>(Name, Size);
	}
}

#endif
