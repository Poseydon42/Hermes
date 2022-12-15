#pragma once

#include <string>
#include <stdint.h>

#include "Core/Compiler/CompilerMSVC.h"

#ifdef HERMES_BUILD_ENGINE
#define HERMES_API API_EXPORT
#define APP_API API_IMPORT
#elif defined(HERMES_BUILD_APPLICATION)
#define HERMES_API API_IMPORT
#define APP_API API_EXPORT
#elif defined(HERMES_BUILD_TOOLS)
#define HERMES_API
#define APP_API
#endif

namespace Hermes
{
	using String = std::string;
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
}

#ifndef HERMES_RELEASE
#define HERMES_ASSERT(Expression) {if(!(Expression)) DEBUG_BREAK(); }
#else
// NOTE : the compiler will *hopefully* optimize it away, but this way we won't get any errors
//        about unreferenced variables (that were only referenced inside the assert expression
#define HERMES_ASSERT(Expression) {if(!(Expression)) {} }
#endif

#define HERMES_ASSERT_LOG(Expression, Msg, ...) {if(!(Expression)) { HERMES_LOG_ERROR(Msg, __VA_ARGS__); DEBUG_BREAK(); } }
