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

		~ApplicationLoop() = default;
		ApplicationLoop(ApplicationLoop&&) = default;
		ApplicationLoop& operator=(ApplicationLoop&&) = default;

		void Run();

		void RequestExit();
		
	private:
		void WindowCloseEventHandler(const IEvent& Event);

		bool RequestedExit;
		
		std::unique_ptr<IApplication> Application;

		std::shared_ptr<IPlatformWindow> ApplicationWindow;
	};

	extern ApplicationLoop* GGameLoop;
}
