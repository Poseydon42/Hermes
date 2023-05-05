#include "InputController.h"

#include "Core/Event/EventQueue.h"

#define BIND_CALLBACK(Func, EventClass) [this](const IEvent& Event) { Func(static_cast<const EventClass&>(Event)); }

namespace Hermes::UI
{
	InputController::InputController(EventQueue& WindowEventQueue, std::weak_ptr<Widget> InRootWidget)
		: RootWidget(std::move(InRootWidget))
	{
		WindowEventQueue.Subscribe(WindowMouseButtonEvent::GetStaticType(), BIND_CALLBACK(MouseButtonEventCallback, WindowMouseButtonEvent));
	}

	void InputController::MouseButtonEventCallback(const WindowMouseButtonEvent& Event)
	{
		auto LowestWidget = GetLowestWidgetAtCoordinates(Event.GetCursorCoordinates());
		if (!LowestWidget)
			return;

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

	Widget* InputController::GetLowestWidgetAtCoordinates(Vec2i Coordinates) const
	{
		auto LockedRootWidget = RootWidget.lock();
		if (!LockedRootWidget->GetBoundingBox().Contains(Coordinates))
			return nullptr;

		auto* CurrentWidget = LockedRootWidget.get();
		while (CurrentWidget)
		{
			bool FoundLowerWidget = false;
			CurrentWidget->ForEachChild([&](Widget& Child)
			{
				if (!Child.GetBoundingBox().Contains(Coordinates))
					return;
				CurrentWidget = &Child;
				FoundLowerWidget = true;
			});

			if (!FoundLowerWidget)
				break;
		}
		return CurrentWidget;
	}
}
