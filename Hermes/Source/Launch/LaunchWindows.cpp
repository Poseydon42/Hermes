#ifdef HERMES_PLATFORM_WINDOWS

#include <Windows.h>
#include <string>

#include "Core/Core.h"
#include "ApplicationCore/Application.h"
#include "ApplicationCore/GameLoop.h"

int WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	std::wstring GameModuleName = TEXT(HERMES_GAME_NAME);
	GameModuleName += L".dll";
	HMODULE GameModule = LoadLibrary(GameModuleName.c_str());
	if (GameModule)
	{
		auto* CreateApplicationInstanceImpl = reinterpret_cast<Hermes::CreateApplicationInstance>(
			GetProcAddress(GameModule, "CreateApplicationInstance"));
		if (CreateApplicationInstanceImpl)
		{
			Hermes::IApplication* App = CreateApplicationInstanceImpl();
			Hermes::GGameLoop = new Hermes::GameLoop(App);
			if (Hermes::GGameLoop->Init())
			{
				Hermes::GGameLoop->Run();
			}
		}
		else
		{
			MessageBoxW(0, L"Failed to get address of function CreateApplicationInstance", L"Error loading game module", MB_ICONHAND | MB_OK);
		}
	}
	else
	{
		std::wstring ErrorText = L"Failed to load ";
		ErrorText += GameModuleName;
		MessageBoxW(0, ErrorText.c_str(), L"Error loading game module", MB_ICONHAND | MB_OK);
	}

	return 0;
}

#endif
