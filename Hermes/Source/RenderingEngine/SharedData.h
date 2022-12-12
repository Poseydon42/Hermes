/*
 * This file defines structs that are used in both engine and shaders code. Therefore, the style might
 * be quite different from the default because we need to take into account the fact that this file will
 * be included in the shader code and that GLSL compiler has limited functionality compared to C++ compilers
 *
 * NOTE: our current GLSL compilation setup defines _GLSL_ when compiling GLSL files, so we can use it to
 *       slightly modify the exact structure of this file depending on what includes it.
 */

// ReSharper disable CppClangTidyCppcoreguidelinesProTypeMemberInit
#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#ifndef _GLSL_

#include "Core/Core.h"
#include "Math/Math.h"

#define ALIGNAS_16 alignas(16)

#else

#define uint32 uint
#define Vec4 vec4
#define Mat4 mat4

#define ALIGNAS_16

#endif

#ifndef _GLSL_
namespace Hermes
{
#endif

	struct ALIGNAS_16 PointLight
	{
		/* Only first 3 components are meaningful, 4th is added for alignment purposes */
		Vec4 Position;
		Vec4 Color;
	};

	struct ALIGNAS_16 GlobalDrawcallData
	{
		Mat4 ModelMatrix;
	};

	struct ALIGNAS_16 GlobalSceneData
	{
#define MAX_POINT_LIGHT_COUNT 1024
#ifndef _GLSL_
		static constexpr uint32 MaxPointLightCount = MAX_POINT_LIGHT_COUNT;
#else
#define MaxPointLightCount MAX_POINT_LIGHT_COUNT
#endif

		Mat4 ViewProjection;
		Vec4 CameraLocation;
		PointLight PointLights[MaxPointLightCount];
		uint32 PointLightCount;

#undef MAX_POINT_LIGHT_COUNT
	};

#ifndef _GLSL_
}
#endif

#endif
