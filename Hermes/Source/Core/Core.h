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
	/**
	 * Hermes is UTF-16 only game engine
	 * ANSI strings are supported only for accessing 3rd party code and API
	 */
	using String = std::wstring;
	using ANSIString = std::string;

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
#define HERMES_ASSERT(Expression)
#endif

#define HERMES_ASSERT_LOG(Expression, Msg, ...) {if(!(Expression)) { HERMES_LOG_ERROR(Msg, __VA_ARGS__); DEBUG_BREAK(); } }
