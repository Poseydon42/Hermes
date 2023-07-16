#pragma once

#include <memory>

#include "Core/Core.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	class HERMES_API InputController
	{
	public:
		InputController(EventQueue& WindowEventQueue, std::weak_ptr<Widget> InRootWidget);

	private:
		std::weak_ptr<Widget> RootWidget;

		std::weak_ptr<Widget> WidgetInFocus;

		void MouseMoveEventCallback(const WindowMouseMoveEvent& Event);

		void MouseButtonEventCallback(const WindowMouseButtonEvent& Event);

		void KeyboardEventCallback(const WindowKeyboardEvent& Event);

		std::shared_ptr<Widget> GetLowestWidgetAtCoordinates(Vec2i Coordinates) const;
	};
}
