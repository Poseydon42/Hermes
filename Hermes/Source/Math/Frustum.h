#pragma once

#include "Core/Core.h"
#include "Math/Matrix.h"
#include "Math/Plane.h"

namespace Hermes
{
	struct SphereBoundingVolume;

	/*
	 * Stores 6 planes that together bound a chunk of space that is visible to the camera and has a shape of a frustum
	 */
	struct HERMES_API Frustum
	{
		Plane Near;
		Plane Far;
		Plane Right;
		Plane Left;
		Plane Top;
		Plane Bottom;

		bool IsInside(const SphereBoundingVolume& Object, const Mat4& ObjectTransform) const;
	};
}
