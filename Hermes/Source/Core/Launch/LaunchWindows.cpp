#ifdef HERMES_PLATFORM_WINDOWS

#include <windows.h>
#include <string>

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Core/Application/GameLoop.h"

int WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	std::wstring GameModuleName = TEXT(HERMES_GAME_NAME);
	GameModuleName += L".dll";
	HMODULE GameModule = LoadLibrary(GameModuleName.c_str());
	if (GameModule)
	{
		Hermes::CreateApplicationInstance CreateApplicationInstanceImpl = (Hermes::CreateApplicationInstance)GetProcAddress(GameModule, "CreateApplicationInstance");
		if (CreateApplicationInstanceImpl)
		{
			Hermes::IApplication* App = CreateApplicationInstanceImpl();
			Hermes::ApplicationLoop AppLoop(App);
			AppLoop.Run();
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
