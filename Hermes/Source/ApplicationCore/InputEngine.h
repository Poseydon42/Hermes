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
		explicit InputEngine(IPlatformWindow& PlatformWindow);

		void Enable();
		void Disable();

		void ProcessDeferredEvents();

		EventQueue& GetEventQueue();

		bool IsKeyPressed(KeyCode Key) const;

		void SetDeltaMousePosition(Vec2 Position);
		Vec2 GetDeltaMousePosition() const;
	private:
		EventQueue Queue;

		bool IsEnabled = false;

		bool KeyState[static_cast<int16>(KeyCode::Count_)] = {};
		Vec2 DeltaMousePosition;

		static constexpr float MouseSensitivity = 0.01f;

		void KeyEventHandler(const IEvent& Event);
		void MouseEventHandler(const IEvent& Event);
	};
}
