#include "GameLoop.h"


#include "EventQueue.h"
#include "Core/Log/Logger.h"
#include "Core/Log/DebugLogDevice.h"
#include "Core/Log/FileLogDevice.h"
#include "Platform/GenericPlatform/PlatformWindow.h"

namespace Hermes
{
	GameLoop* GGameLoop = 0;
	
	GameLoop::GameLoop(IApplication* App) : RequestedExit(false)
	{
		Logger::SetLogLevel(LogLevel::Debug);
		Logger::SetLogFormat(L"[%Y-%M-%d %h:%m:%s:%u][%f:%#][%l] %v");
		Logger::AttachLogDevice(new DebugLogDevice());
		Logger::AttachLogDevice(new FileLogDevice(L"TestLog.log", LogLevel::Info));
		
		HERMES_LOG_INFO(L"Initializing game loop!");

		Application = std::unique_ptr<IApplication>(App);
	}

	bool GameLoop::Init()
	{
		ApplicationWindow = IPlatformWindow::CreatePlatformWindow(L"Hermes Engine", { 1280, 720 });
		if (!ApplicationWindow->IsValid())
			return false;
		if (auto WindowMessageQueue = ApplicationWindow->WindowQueue().lock())
		{
			WindowMessageQueue->Subscribe<GameLoop, &GameLoop::WindowCloseEventHandler>(WindowCloseEvent::GetStaticType(), this);
		}
		else
		{
			return false;
		}
		if (!Application->Init())
		{
			HERMES_LOG_FATAL(L"Application::Init() returned false. Exiting");
			return false;
		}

		return true;
	}

	void GameLoop::Run()
	{
		while (!RequestedExit)
		{
			ApplicationWindow->Run();
			Application->Run(0.0f);
		}
		Application->Shutdown();
	}

	void GameLoop::RequestExit()
	{
		HERMES_LOG_INFO(L"Game loop received exit request");
		RequestedExit = true;
	}

	std::shared_ptr<const IPlatformWindow> GameLoop::GetWindow() const
	{
		return ApplicationWindow;
	}

	void GameLoop::WindowCloseEventHandler(const IEvent& Event)
	{
		HERMES_LOG_INFO(L"Window %S requested exit.", Event.ToString().c_str());

		RequestedExit = true;
	}
}
