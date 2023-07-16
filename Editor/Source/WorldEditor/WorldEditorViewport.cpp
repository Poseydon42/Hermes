#include "WorldEditorViewport.h"

#include <utility>

#include "ApplicationCore/GameLoop.h"

namespace Hermes::Editor
{
	std::shared_ptr<WorldEditorViewport> WorldEditorViewport::Create()
	{
		return std::shared_ptr<WorldEditorViewport>(new WorldEditorViewport);
	}

	void WorldEditorViewport::OnUpdate(float DeltaTime)
	{
		if (InputEngine::IsMouseButtonPressed(MouseButton::Right))
			Camera->Update(DeltaTime);
	}

	WorldEditorViewport::WorldEditorViewport()
		: Camera(std::make_shared<WorldEditorCamera>(Vec3(0.0f), 0.0f, 0.0f))
	{
		GGameLoop->OverrideCamera(Camera);
	}
}
