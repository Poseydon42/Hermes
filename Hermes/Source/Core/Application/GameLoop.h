#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	class IEvent;
	class IPlatformWindow;

	class HERMES_API GameLoop
	{
		MAKE_NON_COPYABLE(GameLoop)
		
	public:
		GameLoop(IApplication* App);

		~GameLoop() = default;
		GameLoop(GameLoop&&) = default;
		GameLoop& operator=(GameLoop&&) = default;

		bool Init();
		
		void Run();

		void RequestExit();
		
		std::shared_ptr<const IPlatformWindow> GetWindow() const;
	private:
		void WindowCloseEventHandler(const IEvent& Event);

		bool RequestedExit;
		
		std::unique_ptr<IApplication> Application;

		std::shared_ptr<IPlatformWindow> ApplicationWindow;
	};

	HERMES_API extern GameLoop* GGameLoop;
}
