#include "GameLoop.h"

#include "Core/Event/EventQueue.h"
#include "Core/Profiling.h"
#include "ApplicationCore/InputEngine.h"
#include "Logging/Logger.h"
#include "Logging/DebugLogDevice.h"
#include "Logging/FileLogDevice.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "RenderingEngine/Renderer.h"
#include "VirtualFilesystem/DirectoryFSDevice.h"
#include "VirtualFilesystem/VirtualFilesystem.h"
#include "World/Systems/LightRenderingSystem.h"
#include "World/Systems/MeshRenderingSystem.h"

namespace Hermes
{
	GameLoop* GGameLoop = nullptr;
	
	GameLoop::GameLoop(IApplication* App)
		: PrevFrameEndTimestamp{}
	{
		VirtualFilesystem::Mount("/", MountMode::ReadOnly, 0, std::make_unique<DirectoryFSDevice>("Hermes/Files"));
		VirtualFilesystem::Mount("/", MountMode::ReadOnly, 1, std::make_unique<DirectoryFSDevice>(String(HERMES_GAME_NAME) + "/Files"));

		Logger::SetLogLevel(LogLevel::Debug);
		Logger::SetLogFormat("[%Y-%M-%d %h:%m:%s:%u][%f:%#][%l] %v");
		Logger::AttachLogDevice(new DebugLogDevice());
		Logger::AttachLogDevice(new FileLogDevice("/log.log", LogLevel::Info));
		
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

		InputEngine = std::make_unique<class InputEngine>(*ApplicationWindow);
		ApplicationWindow->SetCursorVisibility(false);

		ApplicationWindow->GetWindowQueue().Subscribe(WindowCloseEvent::GetStaticType(), [this](const IEvent&) { RequestedExit = true; });

		// DEBUG ONLY
		InputEngine->GetEventQueue().Subscribe(KeyEvent::GetStaticType(), [this](const IEvent& Event) { KeyEventHandler(Event); });

		HERMES_ASSERT_LOG(Renderer::Init(), "Failed to initialize the renderer");

		GameWorld = std::make_unique<World>();

		if (!Application->Init())
		{
			HERMES_LOG_FATAL("Application::Init() returned false. Exiting");
			return false;
		}

		PrevFrameEndTimestamp = PlatformTime::GetCurrentTimestamp();

		// FIXME: this should be done automatically
		GameWorld->AddSystem(std::make_unique<MeshRenderingSystem>());
		GameWorld->AddSystem(std::make_unique<LightRenderingSystem>());

		return true;
	}

	void GameLoop::Run()
	{
		while (!RequestedExit)
		{
			HERMES_PROFILE_SCOPE("GameLoop");
			ApplicationWindow->Run();
			if (!Paused && !RequestedExit)
			{
				auto CurrentTimestamp = PlatformTime::GetCurrentTimestamp();
				float DeltaTime = PlatformTime::ToSeconds({ .Start = PrevFrameEndTimestamp, .End = CurrentTimestamp });

				TotalTime += DeltaTime;
				NumFrames++;

				GameWorld->Update(DeltaTime);

				Application->Run(DeltaTime);
				InputEngine->ProcessDeferredEvents(); // TODO : implement properly(input should be before update rather than after)

				auto& Scene = GameWorld->GetScene();

				Renderer::RunFrame(Scene, *RootWidget);

				PrevFrameEndTimestamp = CurrentTimestamp;
			}
		}

		HERMES_LOG_INFO("Average time per frame: %f", static_cast<double>(TotalTime) / static_cast<double>(NumFrames));

		Application->Shutdown();
		Renderer::Shutdown();
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

	const UI::Widget& GameLoop::GetRootWidget() const
	{
		return *RootWidget;
	}

	void GameLoop::SetRootWidget(std::shared_ptr<UI::Widget> NewRootWidget)
	{
		RootWidget = std::move(NewRootWidget);
	}

	std::shared_ptr<const IPlatformWindow> GameLoop::GetWindow() const
	{
		return ApplicationWindow;
	}

	InputEngine& GameLoop::GetInputEngine()
	{
		return *InputEngine;
	}

	World& GameLoop::GetWorld()
	{
		return *GameWorld;
	}

	void GameLoop::SetCamera(std::shared_ptr<Hermes::Camera> NewCamera)
	{
		Camera = std::move(NewCamera);
	}

	/*
	 * DEBUG ONLY
	 */
	void GameLoop::KeyEventHandler(const IEvent& RawEvent)
	{
		const auto& Event = static_cast<const KeyEvent&>(RawEvent);
		if (Event.IsPressEvent() && Event.GetKeyCode() == KeyCode::F)
		{
			ApplicationWindow->ToggleFullscreen(!IsFullscreen);
			IsFullscreen = !IsFullscreen;
		}
	}
}
