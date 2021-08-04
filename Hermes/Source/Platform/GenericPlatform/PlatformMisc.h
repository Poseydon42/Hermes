#pragma once

#include "Core/Core.h"

namespace Hermes
{
	struct HERMES_API PlatformMisc
	{
		static void Exit(uint32 ExitCode);

		static void ExitWithMessageBox(uint32 ExitCode, const String& Title, const String& Message);
	};
}
