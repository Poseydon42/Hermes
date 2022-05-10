#pragma once

#include "Core/Core.h"
#include "Math/Vector2.h"
#include <cmath>

namespace Hermes
{
	template<typename InternalType>
	struct Vector3
	{
		InternalType X;
		InternalType Y;
		InternalType Z;

		Vector3(InternalType V = 0.0f);

		Vector3(InternalType NX, InternalType NY, InternalType NZ);

		template<typename OtherType>
		explicit Vector3(Vector3<OtherType> V);

		template<typename OtherVectorType, typename OtherValueType>
		Vector3(Vector2<OtherVectorType> NXY, OtherValueType NZ);
		
		/**
		 * Bitwise operators
		 */
		Vector3 operator+(const Vector3& V) const;
		Vector3 operator-(const Vector3& V) const;
		Vector3 operator*(const Vector3& V) const;
		Vector3 operator/(const Vector3& V) const;
		Vector3 operator+(InternalType B) const;
		Vector3 operator-(InternalType B) const;
		Vector3 operator*(InternalType B) const;
		Vector3 operator/(InternalType B) const;
		Vector3& operator+=(const Vector3& V);
		Vector3& operator-=(const Vector3& V);
		Vector3& operator*=(const Vector3& V);
		Vector3& operator/=(const Vector3& V);
		Vector3& operator+=(InternalType B);
		Vector3& operator-=(InternalType B);
		Vector3& operator*=(InternalType B);
		Vector3& operator/=(InternalType B);

		bool operator==(const Vector3& V) const;
		bool operator!=(const Vector3& V) const;

		InternalType& operator[](size_t Index);
		InternalType operator[](size_t Index) const;

		/**
		 * Cross product
		 */
		Vector3 operator^(const Vector3& V) const;

		/**
		 * Dot product
		 */
		InternalType operator|(const Vector3& V) const;

		/**
		 * Negate(flip) the vector
		 */
		Vector3 operator-() const;

		InternalType Length() const;

		InternalType LengthSq() const;

		/**
		 * Normalizes a vector and returns reference to itself
		 */
		Vector3& Normalize();

		/*
		 * Normalizes a vector only if at least one of its components is greater than zero
		 * with a tolerance of Epsilon and returns reference to itself
		 */
		Vector3& SafeNormalize(InternalType Epsilon = InternalType(FLT_EPSILON));

		/*
		 * Returns true if all components of vector are close to zero with a tolerance of Epsilon
		 */
		bool IsCloseToZero(InternalType Epsilon = InternalType(FLT_EPSILON)) const;
	};

	/**
	 * Aliases for vectors of basic types
	 */
	using Vec3 = Vector3<float>;
	using Vec3i = Vector3<int32>;
	using Vec3l = Vector3<int64>;
	using Vec3d = Vector3<double>;

	template <typename InternalType>
	Vector3<InternalType>::Vector3(InternalType V) : X(V), Y(V), Z(V) {}

	template <typename InternalType>
	Vector3<InternalType>::Vector3(InternalType NX, InternalType NY, InternalType NZ) : X(NX), Y(NY), Z(NZ) {}

	template <typename InternalType>
	template <typename OtherType>
	Vector3<InternalType>::Vector3(Vector3<OtherType> V)
		: X(static_cast<InternalType>(V.X))
		, Y(static_cast<InternalType>(V.Y))
		, Z(static_cast<InternalType>(V.Z))
	{
	}

	template <typename InternalType>
	template <typename OtherVectorType, typename OtherValueType>
	Vector3<InternalType>::Vector3(Vector2<OtherVectorType> NXY, OtherValueType NZ)
		: X(static_cast<InternalType>(NXY.X))
		, Y(static_cast<InternalType>(NXY.Y))
		, Z(static_cast<InternalType>(NZ))
	{
	}

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
	Vector3<InternalType> Vector3<InternalType>::operator+(InternalType B) const
	{
		return Vector3(X + B, Y + B, Z + B);
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator-(InternalType B) const
	{
		return *this + (-B);
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator*(InternalType B) const
	{
		return Vector3(X * B, Y * B, Z * B);
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator/(InternalType B) const
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
	Vector3<InternalType>& Vector3<InternalType>::operator+=(InternalType B)
	{
		*this = *this + B;
		return *this;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::operator-=(InternalType B)
	{
		*this = *this - B;
		return *this;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::operator*=(InternalType B)
	{
		*this = *this * B;
		return *this;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::operator/=(InternalType B)
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
	InternalType& Vector3<InternalType>::operator[](size_t Index)
	{
		return (&X)[Index];
	}

	template <typename InternalType>
	InternalType Vector3<InternalType>::operator[](size_t Index) const
	{
		return (&X)[Index];
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
	InternalType Vector3<InternalType>::operator|(const Vector3& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z;
	}

	template <typename InternalType>
	Vector3<InternalType> Vector3<InternalType>::operator-() const
	{
		return Vector3(-X, -Y, -Z);
	}

	template <typename InternalType>
	InternalType Vector3<InternalType>::Length() const
	{
		return sqrt(LengthSq());
	}

	template <typename InternalType>
	InternalType Vector3<InternalType>::LengthSq() const
	{
		return X * X + Y * Y + Z * Z;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::Normalize()
	{
		*this /= Length();
		return *this;
	}

	template <typename InternalType>
	Vector3<InternalType>& Vector3<InternalType>::SafeNormalize(InternalType Epsilon)
	{
		if (!IsCloseToZero(Epsilon))
			return Normalize();
		return *this;
	}

	template <typename InternalType>
	bool Vector3<InternalType>::IsCloseToZero(InternalType Epsilon) const
	{
		return Math::Abs(X) < Epsilon && Math::Abs(Y) < Epsilon && Math::Abs(Z) < Epsilon;
	}
}
