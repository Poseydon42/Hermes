#include "EditorApplication.h"

#include "ApplicationCore/GameLoop.h"
#include "Logging/Logger.h"
#include "RenderingEngine/Renderer.h"
#include "VirtualFilesystem/DirectoryFSDevice.h"
#include "VirtualFilesystem/VirtualFilesystem.h"
#include "World/Components/MeshComponent.h"
#include "World/Components/TransformComponent.h"

namespace Hermes::Editor
{
	bool EditorApplication::EarlyInit()
	{
		VirtualFilesystem::Mount("/Editor", MountMode::ReadOnly, 1, std::make_unique<DirectoryFSDevice>("Editor/Files"));
		VirtualFilesystem::Mount("/", MountMode::ReadWrite, 2, std::make_unique<DirectoryFSDevice>("Editor/Temp"));

		return true;
	}

	bool EditorApplication::Init()
	{
		HERMES_LOG_INFO("Initializing the editor");

		Viewport = WorldEditorViewport::Create();
		GGameLoop->SetRootWidget(Viewport);

		Camera = std::make_shared<WorldEditorCamera>(Vec3(0.0f), 0.0f, 0.0f);
		GGameLoop->OverrideCamera(Camera);

		return true;
	}

	void EditorApplication::Run(float DeltaTime)
	{
		Camera->Update(GGameLoop->GetInputEngine(), DeltaTime);
	}

	void EditorApplication::Shutdown()
	{
		HERMES_LOG_INFO("Shutting down the editor");
	}
	
	extern "C" APP_API IApplication * CreateApplicationInstance()
	{
		auto Application = new EditorApplication;
		return Application;
	}
}
