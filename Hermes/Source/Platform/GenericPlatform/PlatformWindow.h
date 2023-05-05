#pragma once

#include <format>
#include <memory>

#include "Core/Core.h"
#include "Core/Event/Event.h"
#include "Core/Misc/KeyCode.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Math/Vector2.h"

namespace Hermes
{
	class EventQueue;

	class HERMES_API WindowKeyboardEvent : public IEvent
	{
		EVENT_BODY(WindowKeyboardEvent);

	public:
		WindowKeyboardEvent(KeyCode InKeyCode, bool InIsPressEvent)
			: KeyCode(InKeyCode)
			, IsPressEvent(InIsPressEvent)
		{
		}

		virtual String ToString() const override
		{
			return std::format("WindowKeyboardEvent (KeyCode: {}, Type: {})", KeyCodeToString(KeyCode), IsPressEvent ? "pressed" : "released");
		}

		KeyCode GetKeyCode() const
		{
			return KeyCode;
		}

		bool IsPressed() const
		{
			return IsPressEvent;
		}

		bool IsReleased() const
		{
			return !IsPressed();
		}

	private:
		KeyCode KeyCode;
		bool IsPressEvent;
	};

	class HERMES_API WindowMouseMoveEvent : public IEvent
	{
		EVENT_BODY(WindowMouseMoveEvent);

	public:
		explicit WindowMouseMoveEvent(Vec2i InMouseDelta)
			: MouseDelta(InMouseDelta)
		{
		}

		virtual String ToString() const override
		{
			return std::format("WindowMouseMoveEvent (MouseDelta: ({}, {}))", MouseDelta.X, MouseDelta.Y);
		}

		Vec2i GetMouseDelta() const
		{
			return MouseDelta;
		}

	private:
		Vec2i MouseDelta;
	};

	enum class WindowMouseButtonEventType
	{
		Pressed,
		Released
	};

	class HERMES_API WindowMouseButtonEvent : public IEvent
	{
		EVENT_BODY(WindowMouseButtonEvent)

	public:
		WindowMouseButtonEvent(WindowMouseButtonEventType InButtonEventType, MouseButton InButton, Vec2i InCursorCoordinates)
			: ButtonEventType(InButtonEventType)
			, Button(InButton)
			, CursorCoordinates(InCursorCoordinates)
		{
		}

		virtual String ToString() const override
		{
			StringView ButtonEventTypeString;
			switch (ButtonEventType)
			{
			case WindowMouseButtonEventType::Pressed:
				ButtonEventTypeString = "Pressed";
				break;
			case WindowMouseButtonEventType::Released:
				ButtonEventTypeString = "Released";
				break;
			default:
				HERMES_ASSERT(false);
			}
			return std::format("WindowMouseButtonEvent (ButtonEventType: {}, Button: {}, CursorCoordinates: ({}, {}))", ButtonEventTypeString, MouseButtonToString(Button), CursorCoordinates.X, CursorCoordinates.Y);
		}

		WindowMouseButtonEventType GetButtonEventType()
		{
			return ButtonEventType;
		}

		MouseButton GetButton()
		{
			return Button;
		}

		Vec2i GetCursorCoordinates()
		{
			return CursorCoordinates;
		}

	private:
		WindowMouseButtonEventType ButtonEventType;
		MouseButton Button;
		Vec2i CursorCoordinates;
	};

	class HERMES_API WindowStateEvent : public IEvent
	{
		EVENT_BODY(WindowStateEvent)

	public:
		enum class State
		{
			Minimized,
			Maximized
		};

		explicit WindowStateEvent(State InState)
			: State(InState)
		{
		}

		virtual String ToString() const override
		{
			StringView StateString;
			switch (State)
			{
			case State::Maximized:
				StateString = "Maximized";
				break;
			case State::Minimized:
				StateString = "Minimized";
				break;
			default:
				HERMES_ASSERT(false);
			}
			return std::format("WindowStateEvent (State: {})", StateString);
		}

		State GetState() const
		{
			return State;
		}

	private:
		State State;
	};

	class HERMES_API WindowCloseEvent : public IEvent
	{
		EVENT_BODY(WindowCloseEvent)
	
	public:
		explicit WindowCloseEvent(String InName)
			: WindowName(std::move(InName))
		{
		}

		virtual String ToString() const override
		{
			return std::format("WindowCloseEvent (WindowName: {})", WindowName);
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
		virtual void Run() = 0;

		/**
		 * Single queue for all window messages that game loop should handle
		 */
		virtual EventQueue& GetWindowQueue() = 0;

		/**
		 * Returns underlying platform-specific handle
		 */
		virtual void* GetNativeHandle() const = 0;

		/*
		 * Sets whether the cursor should be visible
		 */
		virtual void SetCursorVisibility(bool IsVisible) = 0;

		/*
		 * Returns the current position of the cursor if it is visible, otherwise behaviour is undefined.
		 * The cursor's coordinates are reported in the coordinate system with top left window corner begin
		 * the origin and X and Y axes increasing to the right and downwards. The coordinates might have negative
		 * value or value exceeding the dimensions of the window, which indicates that the cursor is outside of
		 * the window's rectangle and should usually be ignored.
		 */
		virtual Vec2i GetCursorPosition() const = 0;

		static std::unique_ptr<IPlatformWindow> CreatePlatformWindow(const String& Name, Vec2ui Size);
	};
}
