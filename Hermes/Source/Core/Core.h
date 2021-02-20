#pragma once

#include <string>

namespace Hermes
{
	/**
	 * Hermes is UTF-16 only game engine
	 * ANSI strings are supported only for accessing 3rd party code and API
	 */
	using String = std::wstring;
}
