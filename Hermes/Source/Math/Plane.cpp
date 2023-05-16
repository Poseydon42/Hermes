#include "Plane.h"

namespace Hermes
{
	Plane::Plane(Vec3 InNormal, float InW)
		: Normal(InNormal.SafeNormalized())
		, W(InW)
	{
	}

	Plane::Plane(float InX, float InY, float InZ, float InW)
		: Normal(InX, InY, InZ)
		, W(InW)
	{
		Normal = Normal.SafeNormalized();
	}

	Plane::Plane(Vec3 InNormal, Vec3 PointOnPlane)
		: Normal(InNormal.SafeNormalized())
		, W(Normal | PointOnPlane)
	{
	}

	bool Plane::IsValid() const
	{
		return !Normal.IsCloseToZero();
	}
}
