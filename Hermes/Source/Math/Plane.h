#pragma once

#include "Vector.h"
#include "Core/Core.h"

namespace Hermes
{
	/*
	 * Representation of a plane in three dimensions
	 *
	 * Stores the plane in the form Ax + By + Cz = W
	 * A, B and C are the components of the normal vector
	 */
	struct HERMES_API Plane
	{
		Vec3 Normal;
		float W = 0.0f;

		Plane(Vec3 InNormal, float InW);
		Plane(float InX, float InY, float InZ, float InW);
		Plane(Vec3 InNormal, Vec3 PointOnPlane);

		/*
		 * Returns true if the plane is valid (e.g. its normal is not a zero vector)
		 */
		bool IsValid() const;
	};
}
