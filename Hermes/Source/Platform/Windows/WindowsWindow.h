#pragma once

#ifdef HERMES_PLATFORM_WINDOWS

#include <Windows.h>

#include "Core/Core.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Core/Event/EventQueue.h"

namespace Hermes
{
	class HERMES_API WindowsWindow : public IPlatformWindow
	{
		MAKE_NON_COPYABLE(WindowsWindow)
	public:
		WindowsWindow(const String& Name, Vec2ui Size);

		virtual ~WindowsWindow() override;
		
		WindowsWindow(WindowsWindow&& Other);
		
		WindowsWindow& operator=(WindowsWindow&& Other);

		virtual void UpdateName(const String& NewName) override;

		virtual const String& GetName() const override;

		virtual bool ToggleFullscreen(bool Enabled) override;

		virtual bool UpdateVisibility(bool Visible) override;

		virtual bool Resize(Vec2ui NewSize) override;

		virtual Vec2ui GetSize() const override;

		virtual bool IsValid() const override;

		virtual const EventQueue& GetWindowQueue() const override;

		virtual void Run() const override;

		virtual void* GetNativeHandle() const override;

		virtual void SetInputEngine(std::weak_ptr<class InputEngine> InInputEngine) override;

		virtual void SetCursorVisibility(bool IsVisible) override;
	
	private:
		HWND WindowHandle;

		String CurrentName;

		std::unique_ptr<EventQueue> MessagePump;

		WINDOWPLACEMENT PrevPlacement;

		Vec2ui LastKnownSize;

		std::weak_ptr<InputEngine> InputEngine;

		static bool ClassRegistered;

		static constexpr const wchar_t* ClassName = L"HermesWindowClass";

		static constexpr DWORD WindowStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		
		static constexpr DWORD ExStyle = 0;

		bool IsInFocus() const;

		LRESULT MessageHandler(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
		
		static LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
	};
}

#endif
