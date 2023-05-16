#include "BoundingVolume.h"

#include "Plane.h"
#include "Math/Math.h"

namespace Hermes
{
	SphereBoundingVolume::SphereBoundingVolume(float InRadius)
		: Radius(InRadius)
	{
	}

	float SphereBoundingVolume::SignedDistance(const Plane& Plane, const Mat4& WorldTransformationMatrix) const
	{
		Vec4 Origin(0.0f, 0.0f, 0.0f, 1.0f);
		auto SphereWorldLocation4 = WorldTransformationMatrix * Origin;
		auto SphereWorldLocation = Vec3(SphereWorldLocation4.X, SphereWorldLocation4.Y, SphereWorldLocation4.Z);

		// Plane equation: Ax + By + Cz = W
		// Substitute any two of the three coordinates to be 0 and then the third coordinate is equal to W
		// divided by the coefficient of the said coordinate
		HERMES_ASSERT(Plane.IsValid());
		Vec3 RandomPointOnPlane;
		if (Math::Abs(Plane.Normal.Z) > 0.000001f)
		{
			RandomPointOnPlane = { 0.0f, 0.0f, Plane.W / Plane.Normal.Z };
		}
		else if (Math::Abs(Plane.Normal.Y) > 0.000001f)
		{
			RandomPointOnPlane = { 0.0f, Plane.W / Plane.Normal.Y, 0.0f };
		}
		else
		{
			RandomPointOnPlane = { Plane.W / Plane.Normal.X, 0.0f, 0.0f };
		}

		// Since normal is normalized then dot product of these two vectors is the distance between
		// a point on a plane and the sphere location multiplied by the cosine of the angle between
		// them, which is equal to the length of projection of this distance onto a perpendicular to
		// the plane
		float Result = Plane.Normal.Dot(SphereWorldLocation - RandomPointOnPlane);

		return Result;
	}
}
