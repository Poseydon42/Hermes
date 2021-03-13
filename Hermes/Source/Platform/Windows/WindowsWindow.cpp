#include "WindowsWindow.h"

#ifdef HERMES_PLATFORM_WINDOWS

namespace Hermes
{
	bool WindowsWindow::ClassRegistered = false;

	WindowsWindow::WindowsWindow(const String& Name, Vec2i Size)
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

		WindowHandle = CreateWindowExW(
			0, ClassName, Name.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			CW_USEDEFAULT, CW_USEDEFAULT, Size.X, Size.Y,
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

	WindowsWindow::WindowsWindow(WindowsWindow&& Other)
	{
		std::swap(MessagePump, Other.MessagePump);
		std::swap(WindowHandle, Other.WindowHandle);
		std::swap(CurrentName, Other.CurrentName);
	}

	WindowsWindow& WindowsWindow::operator=(WindowsWindow&& Other)
	{
		std::swap(MessagePump, Other.MessagePump);
		std::swap(WindowHandle, Other.WindowHandle);
		std::swap(CurrentName, Other.CurrentName);
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

	bool WindowsWindow::Resize(Vec2i NewSize)
	{
		return SetWindowPos(WindowHandle, HWND_NOTOPMOST, 0, 0, NewSize.X, NewSize.Y, SWP_NOMOVE | SWP_NOZORDER);
	}

	Vec2i WindowsWindow::GetSize() const
	{
		Vec2i Result(0);
		RECT WindowSize;
		if (GetWindowRect(WindowHandle, &WindowSize))
			Result = Vec2i(WindowSize.right - WindowSize.left, WindowSize.bottom - WindowSize.top);
		return Result;
	}

	bool WindowsWindow::IsValid() const
	{
		return WindowHandle != 0;
	}

	std::weak_ptr<EventQueue> WindowsWindow::WindowQueue()
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

	LRESULT WindowsWindow::MessageHandler(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
	{
		switch (Message)
		{
		case WM_DESTROY:
			MessagePump->PushEvent(WindowCloseEvent(GetName()));
			break;
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

	std::shared_ptr<IPlatformWindow> IPlatformWindow::CreatePlatformWindow(const String& Name, Vec2i Size)
	{
		return std::make_shared<WindowsWindow>(Name, Size);
	}
}

#endif
