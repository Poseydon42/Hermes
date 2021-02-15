#ifdef HERMES_PLATFORM_WINDOWS

#include <windows.h>

extern "C" _declspec(dllexport) void InitGame()
{
	OutputDebugStringA("Hello from game\n");
}

#endif