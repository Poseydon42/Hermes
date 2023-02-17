#pragma once

#include "Core/Core.h"
#include "Math/Math.h"

namespace Hermes
{
	struct HERMES_API Transform
	{
		Vec3 Translation = { 0.0f };
		Vec3 Rotation = { 0.0f };
		Vec3 Scale = { 1.0f };

		/*
		 * Builds a transformation matrix for the current transform
		 */
		Mat4 GetTransformationMatrix() const;
	};
}
