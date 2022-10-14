#pragma once

#include "Core/Core.h"
#include "Math/Matrix.h"

namespace Hermes
{
	struct Plane;

	/*
	 * A spherical bounding volume
	 *
	 * Only stores radius, world orientation has to be provided to the collision detection function 
	 */
	struct HERMES_API SphereBoundingVolume
	{
		float Radius;

		explicit SphereBoundingVolume(float InRadius);

		float SignedDistance(const Plane& Plane, const Mat4& WorldTransformationMatrix) const;
	};
}
