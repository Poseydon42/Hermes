#pragma once

#include <optional>

#include "Core/Core.h"
#include "Core/Event/Event.h"
#include "Core/Event/EventQueue.h"
#include "Core/Misc/KeyCode.h"
#include "Math/Vector2.h"
#include "Platform/GenericPlatform/PlatformWindow.h"

namespace Hermes
{
	enum class KeyEventType
	{
		Pressed,
		Released
	};

	class HERMES_API KeyEvent : public IEvent
	{
		EVENT_BODY(KeyEvent)

	public:
		KeyEvent(KeyCode InCode, KeyEventType InEventType);

		virtual String ToString() const override;

		KeyCode GetKeyCode() const;
		KeyEventType GetEventType() const;

		bool IsPressEvent() const;
		bool IsReleaseEvent() const;

		std::optional<wchar_t> TryConvertToChar() const;
	private:
		KeyCode Code;
		KeyEventType EventType;
	};

	/*
	 * Holds and processes all pending input messages from platform layer
	 * Currently is a wrapper around EventQueue, but in future will actually implement more functionality
	 * like key-to-action and key-to-axis mapping etc.
	 */
	class HERMES_API InputEngine
	{
	public:
		static void Init(IPlatformWindow& Window);

		static void ProcessDeferredEvents();

		static void Subscribe(IEvent::EventType Type, std::function<EventQueue::CallbackFunctionPrototype> Callback);

		static bool IsKeyPressed(KeyCode Key);
		static bool IsMouseButtonPressed(MouseButton Button);

		static Vec2 GetCurrentMousePosition();
		static Vec2 GetDeltaMousePosition();

	private:
		static EventQueue Queue;

		static bool KeyState[static_cast<size_t>(KeyCode::Count_)];
		static bool MouseButtonState[static_cast<size_t>(MouseButton::Count_)];
		static Vec2 CurrentMousePosition;
		static Vec2 DeltaMousePosition;

		static constexpr float MouseSensitivity = 0.01f;

		static void KeyEventHandler(const IEvent& Event);
		static void MouseButtonEventHandler(const IEvent& Event);
		static void MouseMoveEventHandler(const IEvent& Event);
	};
}
