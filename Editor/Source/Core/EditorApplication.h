#pragma once

#include "ApplicationCore/Application.h"
#include "Core/Core.h"
#include "WorldEditor/WorldEditorCamera.h"
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
		std::shared_ptr<WorldEditorCamera> Camera;
		std::shared_ptr<WorldEditorViewport> Viewport;
	};
}
