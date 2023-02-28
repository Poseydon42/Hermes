#pragma once

#include <string>
#include <sstream>
#include <stdint.h>

#include "Core/Compiler/CompilerMSVC.h"

#ifdef HERMES_BUILD_ENGINE
#define HERMES_API API_EXPORT
#define APP_API API_IMPORT
#elif defined(HERMES_BUILD_APPLICATION)
#define HERMES_API API_IMPORT
#define APP_API API_EXPORT
#elif defined(HERMES_BUILD_TOOLS) || defined(HERMES_BUILD_TESTS)
#define HERMES_API
#define APP_API
#endif

namespace Hermes
{
	using String = std::string;
	using StringStream = std::stringstream;
	using StringView = std::string_view;

	typedef uint8_t uint8;
	typedef int8_t int8;
	typedef uint16_t uint16;
	typedef int16_t int16;
	typedef uint32_t uint32;
	typedef int32_t int32;
	typedef uint64_t uint64;
	typedef int64_t int64;

	typedef size_t uint64;

	inline void DoAssert [[noreturn]] ()
	{
		DEBUG_BREAK();
		while (true);
	}
}

#define HERMES_ASSERT(Expression) {if(!(Expression)) ::Hermes::DoAssert(); }

#define HERMES_ASSERT_LOG(Expression, Msg, ...) {if(!(Expression)) { HERMES_LOG_ERROR(Msg, __VA_ARGS__); DEBUG_BREAK(); } }
