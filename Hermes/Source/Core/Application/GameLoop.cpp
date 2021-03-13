#include "GameLoop.h"


#include "EventQueue.h"
#include "Core/Log/Logger.h"
#include "Core/Log/DebugLogDevice.h"
#include "Core/Log/FileLogDevice.h"
#include "Platform/GenericPlatform/PlatformWindow.h"

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
		if (!Application->Init())
			return;

		ApplicationWindow = IPlatformWindow::CreatePlatformWindow(L"Hermes Engine", { 1280, 720 });
		if (auto WindowMessageQueue = ApplicationWindow->WindowQueue().lock())
		{
			WindowMessageQueue->Subscribe<ApplicationLoop, &ApplicationLoop::WindowCloseEventHandler>(WindowCloseEvent::GetStaticType(), this);
		}

		RequestedExit = false;
	}

	ApplicationLoop::~ApplicationLoop()
	{
		delete Application;
	}

	void ApplicationLoop::Run()
	{
		while (!RequestedExit)
		{
			ApplicationWindow->Run();
			Application->Run(0.0f);
		}
		Application->Shutdown();
	}

	void ApplicationLoop::WindowCloseEventHandler(const IEvent& Event)
	{
		HERMES_LOG_INFO(L"Window %S requested exit.", Event.ToString().c_str());

		RequestedExit = true;
	}
}
