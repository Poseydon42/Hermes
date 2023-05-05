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

		void MouseButtonEventCallback(const WindowMouseButtonEvent& Event);

		Widget* GetLowestWidgetAtCoordinates(Vec2i Coordinates) const;
	};
}
