#pragma once

#ifdef HERMES_PLATFORM_WINDOWS

#include <windows.h>

#include "Core/Core.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Core/Application/EventQueue.h"

namespace Hermes
{
	class HERMES_API WindowsWindow : public IPlatformWindow
	{
		MAKE_NON_COPYABLE(WindowsWindow)
	public:
		WindowsWindow(const String& Name, Vec2ui Size);
		
		~WindowsWindow() override;
		
		WindowsWindow(WindowsWindow&& Other);
		
		WindowsWindow& operator=(WindowsWindow&& Other);
		
		void UpdateName(const String& NewName) override;
		
		const String& GetName() const override;
		
		bool ToggleFullscreen(bool Enabled) override;
		
		bool UpdateVisibility(bool Visible) override;
		
		bool Resize(Vec2ui NewSize) override;
		
		Vec2ui GetSize() const override;
		
		bool IsValid() const override;

		std::weak_ptr<EventQueue> WindowQueue() const override;

		void Run() const override;
		
		void* GetNativeHandle() const override;
		
		void SetInputEngine(std::weak_ptr<class InputEngine> InInputEngine) override;

		void SetCursorVisibility(bool IsVisible) override;
	
	private:
		HWND WindowHandle;

		String CurrentName;

		std::shared_ptr<EventQueue> MessagePump;

		WINDOWPLACEMENT PrevPlacement;

		Vec2ui LastKnownSize;

		std::weak_ptr<InputEngine> InputEngine;

		static bool ClassRegistered;

		static constexpr const wchar_t* ClassName = L"HermesWindowClass";

		static constexpr DWORD WindowStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		
		static constexpr DWORD ExStyle = 0;

		bool IsInFocus() const;

		LRESULT MessageHandler(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
		
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};
}

#endif
