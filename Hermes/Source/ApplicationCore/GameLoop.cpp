#include "GameLoop.h"

#include "Core/Event/EventQueue.h"
#include "ApplicationCore/InputEngine.h"
#include "Logging/Logger.h"
#include "Logging/DebugLogDevice.h"
#include "Logging/FileLogDevice.h"
#include "Core/Misc/StringUtils.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "RenderInterface/GenericRenderInterface/Instance.h"

namespace Hermes
{
	GameLoop* GGameLoop = nullptr;
	
	GameLoop::GameLoop(IApplication* App)
		: RequestedExit(false)
		, Paused(false)
		, PrevFrameEndTimestamp{}
	{
		PlatformFilesystem::Mount(L"Hermes/Files", L"/", 0);
		PlatformFilesystem::Mount(StringUtils::ANSIToString(HERMES_GAME_NAME) + L"/Files", L"/", 1);

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
		ApplicationWindow->SetCursorVisibility(false);
		if (auto WindowMessageQueue = ApplicationWindow->WindowQueue().lock())
		{
			WindowMessageQueue->Subscribe<GameLoop, &GameLoop::WindowCloseEventHandler>(WindowCloseEvent::GetStaticType(), this);

			// DEBUG ONLY
			InputEngine->GetEventQueue().Subscribe<GameLoop, &GameLoop::KeyEventHandler>(KeyEvent::GetStaticType(), this);
		}
		else
		{
			return false;
		}

		auto RenderInterfaceInstance = RenderInterface::Instance::CreateRenderInterfaceInstance(ApplicationWindow);
		auto GPUName = RenderInterfaceInstance->EnumerateAvailableDevices()[0].Name;
		HERMES_LOG_INFO(L"Using GPU#0: %S", GPUName.c_str());
		auto GPU = RenderInterfaceInstance->GetPhysicalDevice(0);
		Renderer::Get().Init(*GPU);

		GameScene = std::make_unique<Scene>();

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

				auto& Renderer = Renderer::Get();
				Renderer.RunFrame(*GameScene);

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

	Scene& GameLoop::GetScene()
	{
		return *GameScene;
	}

	void GameLoop::WindowCloseEventHandler(const IEvent& Event)
	{
		HERMES_LOG_INFO(L"Window \"%s\" requested exit.", Event.ToString().c_str());

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
