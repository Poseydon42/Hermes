#pragma once

#include <string>

#include "Core/Compiler/CompilerMSVC.h"

namespace Hermes
{
	/**
	 * Hermes is UTF-16 only game engine
	 * ANSI strings are supported only for accessing 3rd party code and API
	 */
	using String = std::wstring;

#ifdef HERMES_BUILD_ENGINE
#define HERMES_API API_EXPORT
#define APP_API API_IMPORT
#elif defined(HERMES_BUILD_APPLICATION)
#define HERMES_API API_IMPORT
#define APP_API API_EXPORT
#endif

}
