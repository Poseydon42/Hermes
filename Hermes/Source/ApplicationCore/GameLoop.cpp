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
		, Application(std::unique_ptr<IApplication>(App))
	{
	}

	bool GameLoop::Init()
	{
		HERMES_PROFILE_THREAD("MainThread");
		PlatformTime::Init();

		VirtualFilesystem::Mount("/", MountMode::ReadOnly, 0, std::make_unique<DirectoryFSDevice>("Hermes/Files"));

		// NOTE: we're not creating any file logging devices since the application might modify the virtual filesystem
		// inside EarlyInit()
		Logger::SetLogLevel(LogLevel::Debug);
		Logger::SetLogFormat("[%Y-%M-%d %h:%m:%s:%u][%f:%#][%l] %v");
		Logger::AttachLogDevice(new DebugLogDevice());

		if (!Application->EarlyInit())
		{
			HERMES_LOG_ERROR("Early application initialization failed");
			return false;
		}

		Logger::AttachLogDevice(new FileLogDevice("/log.log", LogLevel::Info));

		HERMES_LOG_INFO("Initializing game loop!");

		ApplicationWindow = IPlatformWindow::CreatePlatformWindow("Hermes Engine", { 1280, 720 });
		if (!ApplicationWindow->IsValid())
			return false;

		InputEngine = std::make_unique<class InputEngine>(*ApplicationWindow);
		SetInputMode(InputMode::UI);

		ApplicationWindow->GetWindowQueue().Subscribe(WindowCloseEvent::GetStaticType(), [this](const IEvent&) { RequestedExit = true; });
		ApplicationWindow->GetWindowQueue().Subscribe(WindowStateEvent::GetStaticType(), [this](const IEvent& Event)
		{
			const auto& StateEvent = static_cast<const WindowStateEvent&>(Event);
			if (StateEvent.GetState() == WindowStateEvent::State::Maximized)
				Paused = false;
			else
				Paused = true;
		});
		
		HERMES_ASSERT_LOG(Renderer::Init(), "Failed to initialize the renderer");

		GameWorld = std::make_unique<World>();

		if (!Application->Init())
		{
			HERMES_LOG_FATAL("Application::Init() returned false. Exiting");
			return false;
		}

		PrevFrameEndTimestamp = PlatformTime::GetCurrentTimestamp();

		Scene = std::make_unique<class Scene>();

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

				GameWorld->Update(*Scene, DeltaTime);
				UpdateWidgetTree(*RootWidget, DeltaTime);

				Application->Run(DeltaTime);
				InputEngine->ProcessDeferredEvents(); // TODO : implement properly(input should be before update rather than after)

				if (OverridingCamera)
					Scene->ChangeActiveCamera(OverridingCamera);

				Renderer::RunFrame(*Scene, *RootWidget);

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

	const UI::Widget& GameLoop::GetRootWidget() const
	{
		return *RootWidget;
	}

	void GameLoop::SetRootWidget(std::shared_ptr<UI::Widget> NewRootWidget)
	{
		RootWidget = std::move(NewRootWidget);
		UIInputController = std::make_unique<UI::InputController>(ApplicationWindow->GetWindowQueue(), RootWidget);
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

	void GameLoop::SetInputMode(Hermes::InputMode NewMode)
	{
		switch (NewMode)
		{
		case InputMode::Game:
			HERMES_LOG_INFO("Input type switched to Game");
			ApplicationWindow->SetCursorVisibility(false);
			InputEngine->Enable();
			break;
		case InputMode::UI:
			HERMES_LOG_INFO("Input type switched to UI");
			ApplicationWindow->SetCursorVisibility(true);
			InputEngine->Disable();
			break;
		default:
			HERMES_ASSERT(false);
		}

		InputMode = NewMode;
	}

	InputMode GameLoop::GetInputMode() const
	{
		return InputMode;
	}

	void GameLoop::OverrideCamera(std::shared_ptr<Camera> NewCamera)
	{
		OverridingCamera = std::move(NewCamera);
	}

	void GameLoop::UpdateWidgetTree(UI::Widget& Widget, float DeltaTime)
	{
		Widget.ForEachChild([&](UI::Widget& Child) { UpdateWidgetTree(Child, DeltaTime); });
		Widget.OnUpdate(DeltaTime);
	}
}
