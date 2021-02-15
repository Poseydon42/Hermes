#ifdef HERMES_PLATFORM_WINDOWS

#include <windows.h>
#include <string>

typedef void(*InitGame)();

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	std::wstring GameModuleName = TEXT(HERMES_GAME_NAME);
	GameModuleName += L".dll";
	HMODULE GameModule = LoadLibrary(GameModuleName.c_str());
	if (GameModule)
	{
		InitGame Func = (InitGame)GetProcAddress(GameModule, "InitGame");
		if (Func)
		{
			Func();
		}
	}
}
#endif