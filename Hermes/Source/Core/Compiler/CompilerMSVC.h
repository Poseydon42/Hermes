#pragma once

// TODO : premake does not seem to add _MSVC_VER to preprocessor definitions,
// maybe we should do it in a different way if we're ever going to compile
// using other compiler than MSVC on Windows
#ifdef HERMES_PLATFORM_WINDOWS

#define API_EXPORT _declspec(dllexport)
#define API_IMPORT _declspec(dllimport)

#endif