#ifdef HERMES_PLATFORM_WINDOWS

#include <windows.h>

#include "Core/Core.h"
#include "Platform/GenericPlatform/PlatformDebug.h"
#include "Core/Application/Application.h"
#include "Core/Log/Logger.h"
#include "Core/Application/Event.h"

#include "Core/Delegate/Delegate.h"

int Sqr(int a)
{
	return a * a;
}

struct SqrWrapper
{
	int X;
	int GetXMul(int Factor)
	{
		return X * Factor;
	}
public:
	int SqrMul2(int a)
	{
		return a * a * 2;
	}
};

class SandboxApp : public Hermes::IApplication
{
public:
	bool Init() override
	{
		HERMES_LOG_DEBUG(L"Some text, here's an 32 bit hexadecimal integer %#010X and a float %f", 0x1234FFDD, 42.0f);

		Hermes::TDelegate<int, int> Delegate;
		Delegate.Bind<Sqr>();
		HERMES_LOG_DEBUG(L"Delegate result #1: %d", Delegate(5));

		SqrWrapper s;
		Delegate.Bind<SqrWrapper, &SqrWrapper::SqrMul2>(&s);
		HERMES_LOG_DEBUG(L"Delegate result #2: %d", Delegate.Invoke(5));

		s.X = 15;
		Delegate.Bind<SqrWrapper, &SqrWrapper::GetXMul>(&s);
		HERMES_LOG_DEBUG(L"Delegate result #3: %d", Delegate.Invoke(2));

		return true;
	}

	void Run(float Delta) override
	{
		
	}

	void Shutdown() override
	{
		OutputDebugString(TEXT("SandboxApp shutdown\n"));
	}
};

extern "C" _declspec(dllexport) Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
