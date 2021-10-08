#include "GameLoop.h"

#include "Core/Application/EventQueue.h"
#include "Core/Application/InputEngine.h"
#include "Core/Log/Logger.h"
#include "Core/Log/DebugLogDevice.h"
#include "Core/Log/FileLogDevice.h"
#include "Platform/GenericPlatform/PlatformWindow.h"

namespace Hermes
{
	GameLoop* GGameLoop = 0;
	
	GameLoop::GameLoop(IApplication* App)
		: RequestedExit(false)
		, Paused(false)
		, PrevFrameEndTimestamp{}
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
		PlatformTime::Init();

		ApplicationWindow = IPlatformWindow::CreatePlatformWindow(L"Hermes Engine", { 1280, 720 });
		if (!ApplicationWindow->IsValid())
			return false;
		InputEngine = std::make_shared<class InputEngine>();
		ApplicationWindow->SetInputEngine(InputEngine);
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

		PrevFrameEndTimestamp = PlatformTime::GetCurrentTimestamp();

		return true;
	}

	void GameLoop::Run()
	{
		while (!RequestedExit)
		{
			ApplicationWindow->Run();
			if (!Paused && !RequestedExit)
			{
				auto CurrentTimestamp = PlatformTime::GetCurrentTimestamp();
				float DeltaTime = PlatformTime::ToSeconds(CurrentTimestamp - PrevFrameEndTimestamp);

				Application->Run(DeltaTime);
				InputEngine->ProcessDeferredEvents(); // TODO : implement properly(input should be before update rather than after)
				
				PrevFrameEndTimestamp = CurrentTimestamp;
			}
		}
		Application->Shutdown();
	}

	void GameLoop::RequestExit()
	{
		HERMES_LOG_INFO(L"Game loop received exit request");
		RequestedExit = true;
	}

	void GameLoop::SetPause(bool IsPaused)
	{
		HERMES_LOG_INFO(L"Game loop is%s paused now", IsPaused ? L"" : L" not");
		Paused = IsPaused;
	}

	std::shared_ptr<const IPlatformWindow> GameLoop::GetWindow() const
	{
		return ApplicationWindow;
	}

	const InputEngine& GameLoop::GetInputEngine() const
	{
		return *InputEngine;
	}

	void GameLoop::WindowCloseEventHandler(const IEvent& Event)
	{
		HERMES_LOG_INFO(L"Window \"%s\" requested exit.", Event.ToString().c_str());

		RequestedExit = true;
	}
}
