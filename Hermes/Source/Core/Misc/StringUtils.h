#pragma once

#include "Core/Core.h"

namespace Hermes
{
	/**
	 * A collection of different helpful string utilities
	 * All functions are static
	 */
	class HERMES_API StringUtils
	{
	public:
		static String ANSIToString(const ANSIString& In);

		static ANSIString StringToANSI(const String& In);
	};
}
