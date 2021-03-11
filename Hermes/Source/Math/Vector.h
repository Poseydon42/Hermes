#pragma once

#include "Core/Core.h"
#include <cmath>

namespace Hermes
{
	template<typename InternalType>
	struct Vector3
	{
		union
		{
			struct
			{
				float X;
				float Y;
				float Z;
			};
			float E[3];
		};

		Vector3(float V = 0.0f);

		Vector3(float NX, float NY, float NZ);
		
		/**
		 * Bitwise operators
		 */
		Vector3 operator+(const Vector3& V) const;
		Vector3 operator-(const Vector3& V) const;
		Vector3 operator*(const Vector3& V) const;
		Vector3 operator/(const Vector3& V) const;
		Vector3 operator+(float B) const;
		Vector3 operator-(float B) const;
		Vector3 operator*(float B) const;
		Vector3 operator/(float B) const;
		Vector3& operator+=(const Vector3& V);
		Vector3& operator-=(const Vector3& V);
		Vector3& operator*=(const Vector3& V);
		Vector3& operator/=(const Vector3& V);
		Vector3& operator+=(float B);
		Vector3& operator-=(float B);
		Vector3& operator*=(float B);
		Vector3& operator/=(float B);

		bool operator==(const Vector3& V) const;
		bool operator!=(const Vector3& V) const;

		/**
		 * Cross product
		 */
		Vector3 operator^(const Vector3& V) const;

		/**
		 * Dot product
		 */
		float operator|(const Vector3& V) const;

		/**
		 * Negate(flip) the vector
		 */
		Vector3 operator-() const;

		float Length() const;

		float LengthSq() const;

		/**
		 * Normalizes a vector and returns reference to itself
		 */
		Vector3& Normalize();
	};

	/**
	 * Aliases for vectors of basic types
	 */
	using Vec3 = Vector3<float>;
	using Vec3i = Vector3<int32>;
	using Vec3l = Vector3<int64>;
	using Vec3d = Vector3<double>;

	template <typename InternalType>
	Vector3<InternalType>::Vector3(float V) : X(V), Y(V), Z(V) {}

	template <typename InternalType>
	Vector3<InternalType>::Vector3(float NX, float NY, float NZ) : X(NX), Y(NY), Z(NZ) {}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator+(const Vector3& V) const
	{
		return Vector3(X + V.X, Y + V.Y, Z + V.Z);
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator-(const Vector3& V) const
	{
		return *this + (-V);
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator*(const Vector3& V) const
	{
		return Vector3(X * V.X, Y * V.Y, Z * V.Z);
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator/(const Vector3& V) const
	{
		return Vector3(X / V.X, Y / V.Y, Z / V.Z);
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator+(float B) const
	{
		return Vector3(X + B, Y + B, Z + B);
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator-(float B) const
	{
		return *this + (-B);
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator*(float B) const
	{
		return Vector3(X * B, Y * B, Z * B);
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator/(float B) const
	{
		return *this * (1 / B);
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::operator+=(const Vector3& V)
	{
		*this = *this + V;
		return *this;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::operator-=(const Vector3& V)
	{
		*this = *this - V;
		return *this;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::operator*=(const Vector3& V)
	{
		*this = *this * V;
		return *this;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::operator/=(const Vector3& V)
	{
		*this = *this / V;
		return *this;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::operator+=(float B)
	{
		*this = *this + B;
		return *this;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::operator-=(float B)
	{
		*this = *this - B;
		return *this;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::operator*=(float B)
	{
		*this = *this * B;
		return *this;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::operator/=(float B)
	{
		*this = *this / B;
		return *this;
	}

	template <typename InternalType>
	bool Vector3<InternalType>::operator==(const Vector3& V) const
	{
		return (X == V.X) && (Y == V.Y) && (Z == V.Z);
	}

	template <typename InternalType>
	bool Vector3<InternalType>::operator!=(const Vector3& V) const
	{
		return !(*this == V);
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator^(const Vector3& V) const
	{
		return Vector3(
			Y * V.Z - Z * V.Y,
			Z * V.X - X * V.Z,
			X * V.Y - Y * V.X
		);
	}

	template <typename InternalType>
	float Vector3<InternalType>::operator|(const Vector3& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z;
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator-() const
	{
		return Vector3(-X, -Y, -Z);
	}

	template <typename InternalType>
	float Vector3<InternalType>::Length() const
	{
		return sqrt(LengthSq());
	}

	template <typename InternalType>
	float Vector3<InternalType>::LengthSq() const
	{
		return X * X + Y * Y + Z * Z;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::Normalize()
	{
		*this /= Length();
		return *this;
	}
}
