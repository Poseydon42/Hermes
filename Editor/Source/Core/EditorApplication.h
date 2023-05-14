#pragma once

#include "ApplicationCore/Application.h"
#include "Core/Core.h"
#include "RenderingEngine/Scene/Camera.h"

namespace Hermes::Editor
{
	class APP_API EditorApplication : public IApplication
	{
	private:
		virtual bool EarlyInit() override;

		virtual bool Init() override;

		virtual void Run(float Delta) override;

		virtual void Shutdown() override;

	private:
		std::shared_ptr<Camera> Camera;
	};
}
