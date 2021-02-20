#pragma once

#include "Core/Core.h"
#include "Core/Log/ILogDevice.h"
#include "Platform/GenericPlatform/PlatformDebug.h"

namespace Hermes
{
	class DebugLogDevice : public ILogDevice
	{
	public:
		void Write(String Text) override
		{
			PlatformDebug::PrintString(Text);
		}


		void WriteLine(String Text) override
		{
			PlatformDebug::PrintString(Text + L"\n");
		}
	};
}