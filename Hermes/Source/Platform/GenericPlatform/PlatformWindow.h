#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Math/Vector2.h"
#include "Core/Application/Event.h"

namespace Hermes
{
	class InputEngine;
	class EventQueue;

	class HERMES_API WindowCloseEvent : public IEvent
	{
		EVENT_BODY(WindowCloseEvent)
	
	public:
		WindowCloseEvent(const String& Name) : WindowName(Name) {}
		
		String ToString() const override
		{
			return WindowName;
		}
	
	private:
		String WindowName;
	};
	
	/**
	 * Interface to platform windowing API
	 */
	class HERMES_API IPlatformWindow
	{
		MAKE_NON_COPYABLE(IPlatformWindow)
		
	public:
		IPlatformWindow() = default;
		
		virtual ~IPlatformWindow() = default;

		IPlatformWindow(IPlatformWindow&&) = default;

		IPlatformWindow& operator=(IPlatformWindow&&) = default;

		/**
		 * Sets new name for a window
		 */
		virtual void UpdateName(const String& NewName) = 0;

		/**
		 * Returns current window name
		 */
		virtual const String& GetName() const = 0;

		/**
		 * Enables or disables fullscreen mode
		 * @return True if mode has been changed successfully
		 */
		virtual bool ToggleFullscreen(bool Enabled) = 0;

		/**
		 * Updates visibility of a window
		 * @param Visible True if window should be shown
		 * @return True if operation succeeded
		 */
		virtual bool UpdateVisibility(bool Visible) = 0;

		/**
		 * Sets new size of client area of a window
		 * @return True if operation succeeded
		 */
		virtual bool Resize(Vec2ui NewSize) = 0;

		/**
		 * Returns current size of a window
		 */
		virtual Vec2ui GetSize() const = 0;

		/**
		 * True if underlying platform handle is valid and can be used
		 */
		virtual bool IsValid() const = 0;

		/**
		 * Processes window messages and message queue
		 */
		virtual void Run() const = 0;

		/**
		 * Single queue for all window messages that game loop should handle
		 */
		virtual std::weak_ptr<EventQueue> WindowQueue() const = 0;

		/**
		 * Returns underlying platform-specific handle
		 */
		virtual void* GetNativeHandle() const = 0;

		/*
		 * Updates pointer to input engine instance that is used to process window input messages
		 */
		virtual void SetInputEngine(std::weak_ptr<InputEngine> InputEngine) = 0;

		static std::shared_ptr<IPlatformWindow> CreatePlatformWindow(const String& Name, Vec2ui Size);
	};
}
