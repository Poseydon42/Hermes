#pragma once

#ifdef HERMES_PLATFORM_WINDOWS

#	ifdef _MSC_VER

#		define API_EXPORT _declspec(dllexport)
#		define API_IMPORT _declspec(dllimport)

#		define DEBUG_BREAK() __debugbreak();

#		define SUPPRESS_ALL_WARNINGS_BEGIN \
			__pragma(warning(push, 0))

#		define SUPPRESS_ALL_WARNINGS_END \
			__pragma(warning(pop))

#		define PACKED_STRUCT_BEGIN \
			__pragma(pack(push, 1))

#		define PACKED_STRUCT_END \
			__pragma(pack(pop))

#	else
#		error "Hermes does not support compilers other than MSVC yet"
#	endif

#endif
