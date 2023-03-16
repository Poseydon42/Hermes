#pragma once

#include "Core/Core.h"
#include "Math/Vector.h"

namespace Hermes
{
	struct HERMES_API Quaternion
	{
		float W = 1.0f;
		Vec3 XYZ = { 0.0f };

		Quaternion() = default;
		Quaternion(float InW, Vec3 InXYZ);
		Quaternion(float InW, float InX, float InY, float InZ);

		float Length() const;
		float LengthSq() const;

		bool IsNormalized(float Tolerance = 0.00001f) const;

		Quaternion Normalized() const;

		Quaternion operator+(Quaternion Other) const;
		Quaternion operator-(Quaternion Other) const;

		Quaternion operator*(float Scalar) const;
		Quaternion operator*(Quaternion Other) const;

		Quaternion Conjugate() const;
	};
}
