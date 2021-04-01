#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	class IEvent;
	class IPlatformWindow;

	class HERMES_API ApplicationLoop
	{
		MAKE_NON_COPYABLE(ApplicationLoop)
		
	public:
		ApplicationLoop(IApplication* App);

		~ApplicationLoop();

		void Run();
		
	private:
		void WindowCloseEventHandler(const IEvent& Event);

		bool RequestedExit;
		
		IApplication* Application;

		std::shared_ptr<IPlatformWindow> ApplicationWindow;
	};
}
