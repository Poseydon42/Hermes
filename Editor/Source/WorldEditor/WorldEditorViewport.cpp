#include "WorldEditorViewport.h"

#include "ApplicationCore/GameLoop.h"

namespace Hermes::Editor
{
	std::shared_ptr<WorldEditorViewport> WorldEditorViewport::Create()
	{
		return std::shared_ptr<WorldEditorViewport>(new WorldEditorViewport());
	}

	bool WorldEditorViewport::OnMouseDown(MouseButton Button)
	{
		if (Button != MouseButton::Right)
			return false;

		GGameLoop->SetInputMode(InputMode::Game);
		return true;
	}

	bool WorldEditorViewport::OnMouseUp(MouseButton Button)
	{
		if (Button != MouseButton::Right)
			return false;

		GGameLoop->SetInputMode(InputMode::UI);
		return true;
	}
}
