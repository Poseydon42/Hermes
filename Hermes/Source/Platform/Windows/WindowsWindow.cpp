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

		WindowHandle = CreateWindowExW(
			0, ClassName, Name.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			CW_USEDEFAULT, CW_USEDEFAULT, Size.X, Size.Y,
			0, 0, AppInstance, this);
		if (WindowHandle)
			UpdateVisibility(true);
		else
			HERMES_LOG_ERROR(L"Failed to create Win32 window! Error code: %#010X", GetLastError());

		CurrentName = Name;
		MessagePump = std::make_shared<EventQueue>();
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
		return false;
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

	LRESULT WindowsWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		HERMES_LOG_INFO(L"Received window message: %d", uMsg);
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	std::shared_ptr<IPlatformWindow> IPlatformWindow::CreatePlatformWindow(const String& Name, Vec2i Size)
	{
		return std::make_shared<WindowsWindow>(Name, Size);
	}
}

#endif
