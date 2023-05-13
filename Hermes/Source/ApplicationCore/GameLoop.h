#pragma once

#include <memory>

#include "ApplicationCore/Application.h"
#include "ApplicationCore/InputEngine.h"
#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Platform/GenericPlatform/PlatformTime.h"
#include "RenderingEngine/Scene/Scene.h"
#include "UIEngine/InputController.h"
#include "UIEngine/Widgets/Widget.h"
#include "World/World.h"

namespace Hermes
{
	class IPlatformWindow;

	enum class InputMode
	{
		UI,
		Game
	};

	class HERMES_API GameLoop
	{
		MAKE_NON_COPYABLE(GameLoop)
		
	public:
		explicit GameLoop(IApplication* App);

		~GameLoop() = default;
		GameLoop(GameLoop&&) = default;
		GameLoop& operator=(GameLoop&&) = default;

		bool Init();
		
		void Run();

		void RequestExit();

		const UI::Widget& GetRootWidget() const;
		void SetRootWidget(std::shared_ptr<UI::Widget> NewRootWidget);
		
		std::shared_ptr<const IPlatformWindow> GetWindow() const;

		InputEngine& GetInputEngine();

		World& GetWorld();

		void SetInputMode(InputMode NewMode);
		InputMode GetInputMode() const;

		/**
		 * If not null, replaces the current active camera in the scene with the provided camera.
		 * If null, reverts back to using the scene camera. The state is preserved between frames,
		 * so you don't need to call this function every frame.
		 */
		void OverrideCamera(std::shared_ptr<Camera> NewCamera);

	private:
		bool RequestedExit = false;
		bool Paused = false; // TODO : separate 'game pause' and 'rendering pause'(e.g. when window is minimized)

		PlatformTimestamp PrevFrameEndTimestamp;

		std::unique_ptr<IApplication> Application;
		std::shared_ptr<IPlatformWindow> ApplicationWindow;

		std::unique_ptr<InputEngine> InputEngine;
		std::unique_ptr<UI::InputController> UIInputController;
		InputMode InputMode = InputMode::Game;

		std::shared_ptr<Camera> OverridingCamera;

		std::unique_ptr<World> GameWorld;
		std::unique_ptr<Scene> Scene;
		std::shared_ptr<UI::Widget> RootWidget;


		/*
		 * DEBUG ONLY
		 */
		void KeyEventHandler(const WindowKeyboardEvent& Event);
		bool IsFullscreen = false;

		float TotalTime = 0.0f;
		uint32 NumFrames = 0;
	};

	HERMES_API extern GameLoop* GGameLoop;
}
