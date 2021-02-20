#pragma once

#include "Core/Core.h"
#include "Core/Application/Application.h"

namespace Hermes
{
	class ApplicationLoop
	{
	public:
		ApplicationLoop(IApplication* App);

		void Run();

	private:
		IApplication* Application;
	};
}