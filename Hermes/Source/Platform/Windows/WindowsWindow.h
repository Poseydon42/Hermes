#pragma once

#ifdef HERMES_PLATFORM_WINDOWS

#include <windows.h>

#include "Core/Core.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Core/Application/EventQueue.h"

namespace Hermes
{
	class HERMES_API WindowsWindow final : public IPlatformWindow
	{
	public:
		WindowsWindow(const String& Name, Vec2i Size);
		
		~WindowsWindow();
		
		WindowsWindow(WindowsWindow&& Other);
		
		WindowsWindow& operator=(WindowsWindow&& Other);
		
		void UpdateName(const String& NewName) override;
		
		const String& GetName() const override;
		
		bool ToggleFullscreen(bool Enabled) override;
		
		bool UpdateVisibility(bool Visible) override;
		
		bool Resize(Vec2i NewSize) override;
		
		Vec2i GetSize() const override;
		
		bool IsValid() const override;

		std::weak_ptr<EventQueue> WindowQueue() override;
	
	private:
		HWND WindowHandle;

		String CurrentName;

		std::shared_ptr<EventQueue> MessagePump;

		static bool ClassRegistered;

		static constexpr const wchar_t* ClassName = L"HermesWindowClass";

		LRESULT MessageHandler(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
		
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};
}

#endif
