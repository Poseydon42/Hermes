#include "GameLoop.h"

#include "Core/Event/EventQueue.h"
#include "Core/Profiling.h"
#include "ApplicationCore/InputEngine.h"
#include "Logging/Logger.h"
#include "Logging/DebugLogDevice.h"
#include "Logging/FileLogDevice.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "RenderingEngine/Renderer.h"

namespace Hermes
{
	GameLoop* GGameLoop = nullptr;
	
	GameLoop::GameLoop(IApplication* App)
		: RequestedExit(false)
		, Paused(false)
		, PrevFrameEndTimestamp{}
	{
		PlatformFilesystem::Mount("Hermes/Files", "/", 0);
		PlatformFilesystem::Mount(String(HERMES_GAME_NAME) + "/Files", "/", 1);

		Logger::SetLogLevel(LogLevel::Debug);
		Logger::SetLogFormat("[%Y-%M-%d %h:%m:%s:%u][%f:%#][%l] %v");
		Logger::AttachLogDevice(new DebugLogDevice());
		Logger::AttachLogDevice(new FileLogDevice("TestLog.log", LogLevel::Info));
		
		HERMES_LOG_INFO("Initializing game loop!");

		Application = std::unique_ptr<IApplication>(App);
	}

	bool GameLoop::Init()
	{
		HERMES_PROFILE_THREAD("MainThread");
		PlatformTime::Init();

		ApplicationWindow = IPlatformWindow::CreatePlatformWindow("Hermes Engine", { 1280, 720 });
		if (!ApplicationWindow->IsValid())
			return false;

		InputEngine = std::make_shared<class InputEngine>();
		ApplicationWindow->SetInputEngine(InputEngine);
		ApplicationWindow->SetCursorVisibility(false);

		ApplicationWindow->GetWindowQueue().Subscribe<GameLoop, &GameLoop::WindowCloseEventHandler>(WindowCloseEvent::GetStaticType(), this);

		// DEBUG ONLY
		InputEngine->GetEventQueue().Subscribe<GameLoop, &GameLoop::KeyEventHandler>(KeyEvent::GetStaticType(), this);

		Renderer::Get().Init();

		GameScene = std::make_unique<Scene>();

		if (!Application->Init())
		{
			HERMES_LOG_FATAL("Application::Init() returned false. Exiting");
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
				HERMES_PROFILE_SCOPE("GameLoop");
				auto CurrentTimestamp = PlatformTime::GetCurrentTimestamp();
				float DeltaTime = PlatformTime::ToSeconds(CurrentTimestamp - PrevFrameEndTimestamp);

				Application->Run(DeltaTime);
				InputEngine->ProcessDeferredEvents(); // TODO : implement properly(input should be before update rather than after)

				auto& Renderer = Renderer::Get();
				Renderer.RunFrame(*GameScene);

				PrevFrameEndTimestamp = CurrentTimestamp;
			}
		}
		Application->Shutdown();
	}

	void GameLoop::RequestExit()
	{
		HERMES_LOG_INFO("Game loop received exit request");
		RequestedExit = true;
	}

	void GameLoop::SetPause(bool IsPaused)
	{
		HERMES_LOG_INFO("Game loop is%s paused now", IsPaused ? "" : " not");
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

	Scene& GameLoop::GetScene()
	{
		return *GameScene;
	}

	void GameLoop::WindowCloseEventHandler(const IEvent& Event)
	{
		HERMES_LOG_INFO("Window \"%s\" requested exit.", Event.ToString().c_str());

		RequestedExit = true;
	}

	/*
	 * DEBUG ONLY
	 */
	void GameLoop::KeyEventHandler(const IEvent& Event)
	{
		const auto& KeyEvent = static_cast<const class KeyEvent&>(Event);
		if (KeyEvent.IsPressEvent() && KeyEvent.GetKeyCode() == KeyCode::F)
		{
			ApplicationWindow->ToggleFullscreen(!IsFullscreen);
			IsFullscreen = !IsFullscreen;
		}
	}
}
