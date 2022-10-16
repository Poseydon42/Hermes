#include "Frustum.h"

#include "Math/BoundingVolume.h"
#include "Math/Common.h"

namespace Hermes
{
	bool Frustum::IsInside(const SphereBoundingVolume& Object, const Mat4& ObjectTransform) const
	{
		auto CheckSinglePlane = [&](const Plane& Plane) -> bool
		{
			auto Distance = Object.SignedDistance(Plane, ObjectTransform);

			// If unsigned distance is less than or equal to the radius of the object it means that
			// the object intersects the plane, so at least a part of the object is inside the frustum,
			// so we return true
			if (Math::Abs(Distance) <= Object.Radius)
				return true;

			// If signed distance is negative and is greater than the radius of the object it means that
			// the object is outside of the frustum relative to the current plane and cannot be inside it
			// even partially since it is too far away
			if (Distance < -Object.Radius)
				return false;

			return true;
		};

		return CheckSinglePlane(Near) && CheckSinglePlane(Far) &&
			CheckSinglePlane(Left) && CheckSinglePlane(Right) &&
			CheckSinglePlane(Top) && CheckSinglePlane(Bottom);
	}
}
