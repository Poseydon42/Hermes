#pragma once

#include "ApplicationCore/Application.h"
#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/HorizontalContainer.h"
#include "WorldEditor/WorldEditorEntityList.h"
#include "WorldEditor/WorldEditorViewport.h"

namespace Hermes::Editor
{
	class APP_API EditorApplication : public IApplication
	{
	private:
		virtual bool EarlyInit() override;

		virtual bool Init() override;

		virtual void Run(float DeltaTime) override;

		virtual void Shutdown() override;

	private:
		std::shared_ptr<WorldEditorViewport> Viewport;
		std::shared_ptr<WorldEditorEntityList> EntityList;

		std::shared_ptr<UI::HorizontalContainer> RootWidget;
	};
}
