#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyable.h"
#include "Math/Vector2.h"
#include "Core/Application/Event.h"

namespace Hermes
{
	class EventQueue;

	class WindowCloseEvent : public IEvent
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
	
	class HERMES_API IPlatformWindow : public INonCopyable
	{
	public:
		virtual ~IPlatformWindow() = default;

		virtual void UpdateName(const String& NewName) = 0;

		virtual const String& GetName() const = 0;

		virtual bool ToggleFullscreen(bool Enabled) = 0;

		virtual bool UpdateVisibility(bool Visible) = 0;

		virtual bool Resize(Vec2i NewSize) = 0;

		virtual Vec2i GetSize() const = 0;

		virtual bool IsValid() const = 0;

		virtual void Run() const = 0;

		virtual std::weak_ptr<EventQueue> WindowQueue() = 0;

		static std::shared_ptr<IPlatformWindow> CreatePlatformWindow(const String& Name, Vec2i Size);
	};
}
