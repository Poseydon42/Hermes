#include "GameLoop.h"

#include "Core/Log/Logger.h"
#include "Core/Log/DebugLogDevice.h"

namespace Hermes
{
	ApplicationLoop::ApplicationLoop(IApplication* App)
	{
		Logger::SetLogLevel(LogLevel::Debug);
		Logger::SetLogFormat(L"[%Y-%M-%d %h:%m:%s:%u][%f:%#][%l] %v");
		Logger::AttachLogDevice(new DebugLogDevice());
		HERMES_LOG_DEBUG(L"Initializing game loop!");
		Application = App;
		App->Init();
	}

	void ApplicationLoop::Run()
	{
		while (1)
			Application->Run(0.0f);
		Application->Shutdown();
	}
}