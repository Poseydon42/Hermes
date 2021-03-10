#include "GameLoop.h"

#include "Core/Log/Logger.h"
#include "Core/Log/DebugLogDevice.h"
#include "Core/Log/FileLogDevice.h"

namespace Hermes
{
	ApplicationLoop::ApplicationLoop(IApplication* App)
	{
		Logger::SetLogLevel(LogLevel::Debug);
		Logger::SetLogFormat(L"[%Y-%M-%d %h:%m:%s:%u][%f:%#][%l] %v");
		Logger::AttachLogDevice(new DebugLogDevice());
		Logger::AttachLogDevice(new FileLogDevice(L"TestLog.log", LogLevel::Info));
		HERMES_LOG_INFO(L"Initializing game loop!");
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
