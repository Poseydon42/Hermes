#include "Quaternion.h"

#include "Math/Common.h"

namespace Hermes
{
	Quaternion::Quaternion(float InW, Vec3 InXYZ)
		: W(InW)
		, XYZ(InXYZ)
	{
	}

	Quaternion::Quaternion(float InW, float InX, float InY, float InZ)
		: W(InW)
		, XYZ(InX, InY, InZ)
	{
	}

	float Quaternion::Length() const
	{
		return Math::Sqrt(LengthSq());
	}

	float Quaternion::LengthSq() const
	{
		return W * W + XYZ.LengthSq();
	}

	bool Quaternion::IsNormalized(float Tolerance) const
	{
		return Math::Abs(LengthSq() - 1.0f) < Tolerance;
	}

	Quaternion Quaternion::Normalized() const
	{
		auto Length = this->Length();
		return { W / Length, XYZ / Length };
	}

	Quaternion Quaternion::operator+(Quaternion Other) const
	{
		return { W + Other.W, XYZ + Other.XYZ };
	}

	Quaternion Quaternion::operator-(Quaternion Other) const
	{
		return { W - Other.W, XYZ - Other.XYZ };
	}

	Quaternion Quaternion::operator*(float Scalar) const
	{
		return { W * Scalar, XYZ * Scalar };
	}

	Quaternion Quaternion::operator*(Quaternion Other) const
	{
		return { W * Other.W - (XYZ | Other.XYZ), Other.XYZ * W + XYZ * Other.W + (XYZ ^ Other.XYZ) };
	}

	Quaternion Quaternion::Conjugate() const
	{
		return { W, -XYZ };
	}
}
