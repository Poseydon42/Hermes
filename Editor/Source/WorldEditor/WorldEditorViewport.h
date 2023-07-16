#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/ViewportContainer.h"
#include "WorldEditor/WorldEditorCamera.h"

namespace Hermes::Editor
{
	class APP_API WorldEditorViewport : public UI::ViewportContainer
	{
	public:
		static std::shared_ptr<WorldEditorViewport> Create();

		virtual void OnUpdate(float DeltaTime) override;

	private:
		WorldEditorViewport();

		std::shared_ptr<WorldEditorCamera> Camera;
	};
}
