#pragma once

#ifdef HERMES_ENABLE_PROFILING
// NOTE: because Windows.h defines min and max which breaks tracy
#undef max
#undef min
#include <tracy/Tracy.hpp>

#include "Core/Core.h"

/*
 * Add this macro at the top of every function you would like to include in the profile
 */
#define HERMES_PROFILE_FUNC() ZoneScoped

/*
 * Allows you to specify a name for the scope you are profiling, e.g. to have multiple profiling marks in one function
 */
#define HERMES_PROFILE_SCOPE(Name) ZoneScopedN(Name)

/*
 * Allows you to attach a custom value to the current profile scope
 *
 * Integers, single- and double-precision floating point numbers are supported
 */
#define HERMES_PROFILE_TAG(Name, Value)	TracyPlot(Name, Value)

/*
 * Add this macro at the start of every thread to assign a name to it
 */
#define HERMES_PROFILE_THREAD(Name) tracy::SetThreadName(Name)

/*
 * Add this macro right before swapchain flip command to mark the end of a frame
 */
#define HERMES_PROFILE_FRAME() FrameMark

#else

#define HERMES_PROFILE_FUNC(...)
#define HERMES_PROFILE_SCOPE(...)
#define HERMES_PROFILE_TAG(...)
#define HERMES_PROFILE_THREAD(...)
#define HERMES_PROFILE_FRAME(...)

#endif