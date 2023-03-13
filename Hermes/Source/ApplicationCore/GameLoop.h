#pragma once

#include <memory>

#include "Core/Core.h"
#include "ApplicationCore/Application.h"
#include "AssetSystem/AssetCache.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Platform/GenericPlatform/PlatformTime.h"
#include "World/World.h"

namespace Hermes
{
	class InputEngine;
	class IEvent;
	class IPlatformWindow;

	class HERMES_API GameLoop
	{
		MAKE_NON_COPYABLE(GameLoop)
		
	public:
		GameLoop(IApplication* App);

		~GameLoop() = default;
		GameLoop(GameLoop&&) = default;
		GameLoop& operator=(GameLoop&&) = default;

		bool Init();
		
		void Run();

		void RequestExit();

		void SetPause(bool IsPaused);
		
		std::shared_ptr<const IPlatformWindow> GetWindow() const;

		const InputEngine& GetInputEngine() const;

		AssetCache& GetAssetCache();

		World& GetWorld();


		/*
		 * DEBUG ONLY
		 */
		void SetCamera(std::shared_ptr<Camera> NewCamera);

	private:
		void WindowCloseEventHandler(const IEvent& Event);

		bool RequestedExit;
		bool Paused; // TODO : separate 'game pause' and 'rendering pause'(e.g. when window is minimized)

		PlatformTimestamp PrevFrameEndTimestamp;

		std::unique_ptr<IApplication> Application;
		std::shared_ptr<IPlatformWindow> ApplicationWindow;
		std::shared_ptr<InputEngine> InputEngine;

		std::unique_ptr<AssetCache> AssetCache;

		std::unique_ptr<World> GameWorld;

		/*
		 * DEBUG ONLY
		 */
		std::shared_ptr<Camera> Camera;
		void KeyEventHandler(const IEvent& Event);
		bool IsFullscreen = false;
	};

	HERMES_API extern GameLoop* GGameLoop;
}
