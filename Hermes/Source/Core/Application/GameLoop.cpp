#include "GameLoop.h"

namespace Hermes
{
	ApplicationLoop::ApplicationLoop(IApplication* App)
	{
		Application = App;
		App->Init();
	}

	void ApplicationLoop::Run()
	{
		while (1)
			Application->Run(0.0f);
		Application->Shutdown();
	}
}
