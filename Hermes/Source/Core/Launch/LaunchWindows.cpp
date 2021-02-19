#ifdef HERMES_PLATFORM_WINDOWS

#include <windows.h>
#include <string>

#include "Core/Application/Application.h"

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
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
