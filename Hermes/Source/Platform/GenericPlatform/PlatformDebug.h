#pragma once

#include "Core/Core.h"

namespace Hermes
{
	struct HERMES_API PlatformDebug
	{
		/**
		 * Prints string to platform debug output
		 * Not all platform can and will implement this
		 */
		static void PrintString(String Text);
	};
}
