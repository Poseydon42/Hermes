#include "Plane.h"

namespace Hermes
{
	Plane::Plane(Vec3 InNormal, float InW)
		: Normal(InNormal.SafeNormalize())
		, W(InW)
	{
		Normal.SafeNormalize();
	}

	Plane::Plane(float InX, float InY, float InZ, float InW)
		: Normal(InX, InY, InZ)
		, W(InW)
	{
		Normal.SafeNormalize();
	}

	Plane::Plane(Vec3 InNormal, Vec3 PointOnPlane)
		: Normal(InNormal.SafeNormalize())
		, W(Normal | PointOnPlane)
	{
	}

	bool Plane::IsValid() const
	{
		return !Normal.IsCloseToZero();
	}
}
