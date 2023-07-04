#include "InputController.h"

#include "Core/Event/EventQueue.h"

#define BIND_CALLBACK(Func, EventClass) [this](const IEvent& Event) { Func(static_cast<const EventClass&>(Event)); }

namespace Hermes::UI
{
	InputController::InputController(EventQueue& WindowEventQueue, std::weak_ptr<Widget> InRootWidget)
		: RootWidget(std::move(InRootWidget))
	{
		WindowEventQueue.Subscribe(WindowMouseButtonEvent::GetStaticType(), BIND_CALLBACK(MouseButtonEventCallback, WindowMouseButtonEvent));
		WindowEventQueue.Subscribe(WindowKeyboardEvent::GetStaticType(), BIND_CALLBACK(KeyboardEventCallback, WindowKeyboardEvent));
	}

	void InputController::MouseButtonEventCallback(const WindowMouseButtonEvent& Event)
	{
		auto LowestWidget = GetLowestWidgetAtCoordinates(Event.GetCursorCoordinates());
		if (!LowestWidget)
			return;

		WidgetInFocus = LowestWidget;

		auto CurrentWidget = LowestWidget;
		while (CurrentWidget)
		{
			auto HasHandledEvent = false;
			if (Event.GetButtonEventType() == WindowMouseButtonEventType::Pressed)
				HasHandledEvent = CurrentWidget->OnMouseDown(Event.GetButton());
			else if (Event.GetButtonEventType() == WindowMouseButtonEventType::Released)
				HasHandledEvent = CurrentWidget->OnMouseUp(Event.GetButton());

			if (HasHandledEvent)
				return;

			CurrentWidget = CurrentWidget->GetParent();
		}
	}

	void InputController::KeyboardEventCallback(const WindowKeyboardEvent& Event)
	{
		if (WidgetInFocus.expired())
			return;

		auto LockedWidgetInFocus = WidgetInFocus.lock();
		if (!LockedWidgetInFocus)
			return;
		
		if (Event.IsPressed())
			LockedWidgetInFocus->OnKeyDown(Event.GetKeyCode(), Event.GetUnicodeCodepoint());
		else if (Event.IsReleased())
			LockedWidgetInFocus->OnKeyUp(Event.GetKeyCode(), Event.GetUnicodeCodepoint());
	}

	std::shared_ptr<Widget> InputController::GetLowestWidgetAtCoordinates(Vec2i Coordinates) const
	{
		auto LockedRootWidget = RootWidget.lock();
		if (!LockedRootWidget->GetBoundingBox().Contains(Coordinates))
			return nullptr;

		auto CurrentWidget = LockedRootWidget;
		while (CurrentWidget)
		{
			bool FoundLowerWidget = false;
			CurrentWidget->ForEachChild([&](std::shared_ptr<Widget> Child)
			{
				if (!Child->GetBoundingBox().Contains(Coordinates))
					return;
				CurrentWidget = std::move(Child);
				FoundLowerWidget = true;
			});

			if (!FoundLowerWidget)
				break;
		}
		return CurrentWidget;
	}
}
