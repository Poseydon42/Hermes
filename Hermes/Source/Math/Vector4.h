#pragma once

#include "Core/Core.h"
#include "Math/Common.h"
#include "Math/Vector.h"
#include <cmath>

namespace Hermes
{
	template<typename InternalType>
	struct Vector4
	{
		InternalType X;
		InternalType Y;
		InternalType Z;
		InternalType W;

		constexpr Vector4(InternalType V = 0.0f);

		constexpr Vector4(InternalType NX, InternalType NY, InternalType NZ, InternalType NW);

		template<typename OtherType>
		explicit constexpr Vector4(Vector4<OtherType> Other);

		template<typename OtherType>
		constexpr Vector4(Vector3<OtherType> V3, OtherType InW);
		
		/**
		 * Bitwise operators
		 */
		constexpr Vector4 operator+(const Vector4& V) const;
		constexpr Vector4 operator-(const Vector4& V) const;
		constexpr Vector4 operator*(const Vector4& V) const;
		constexpr Vector4 operator/(const Vector4& V) const;
		constexpr Vector4 operator+(InternalType B) const;
		constexpr Vector4 operator-(InternalType B) const;
		constexpr Vector4 operator*(InternalType B) const;
		constexpr Vector4 operator/(InternalType B) const;
		constexpr Vector4& operator+=(const Vector4& V);
		constexpr Vector4& operator-=(const Vector4& V);
		constexpr Vector4& operator*=(const Vector4& V);
		constexpr Vector4& operator/=(const Vector4& V);
		constexpr Vector4& operator+=(InternalType B);
		constexpr Vector4& operator-=(InternalType B);
		constexpr Vector4& operator*=(InternalType B);
		constexpr Vector4& operator/=(InternalType B);

		constexpr bool operator==(const Vector4& V) const;
		constexpr bool operator!=(const Vector4& V) const;

		constexpr InternalType& operator[](size_t Index);
		constexpr InternalType operator[](size_t Index) const;

		/**
		 * Dot product
		 */
		constexpr InternalType operator|(const Vector4& V) const;

		/**
		 * Negate(flip) the vector
		 */
		constexpr Vector4 operator-() const;

		constexpr InternalType Length() const;

		constexpr InternalType LengthSq() const;

		/**
		 * Normalizes a vector and returns reference to itself
		 */
		constexpr Vector4& Normalize();

		/*
		 * Normalizes a vector only if at least one of its components is greater than zero
		 * with a tolerance of Epsilon and returns reference to itself
		 */
		constexpr Vector4& SafeNormalize(InternalType Epsilon = InternalType(FLT_EPSILON));

		/*
		 * Returns true if all components of vector are close to zero with a tolerance of Epsilon
		 */
		constexpr bool IsCloseToZero(InternalType Epsilon = InternalType(FLT_EPSILON)) const;
	};

	/**
	 * Aliases for vectors of basic types
	 */
	using Vec4 = Vector4<float>;
	using Vec4i = Vector4<int32>;
	using Vec4l = Vector4<int64>;
	using Vec4d = Vector4<double>;

	template <typename InternalType>
	constexpr Vector4<InternalType>::Vector4(InternalType V) : X(V), Y(V), Z(V), W(V) {}

	template <typename InternalType>
	constexpr Vector4<InternalType>::Vector4(InternalType NX, InternalType NY, InternalType NZ, InternalType NW)
		: X(NX)
		, Y(NY)
		, Z(NZ)
		, W(NW) {}

	template <typename InternalType>
	template <typename OtherType>
	constexpr Vector4<InternalType>::Vector4(Vector4<OtherType> Other)
		: X(static_cast<InternalType>(Other.X))
		, Y(static_cast<InternalType>(Other.Y))
		, Z(static_cast<InternalType>(Other.Z))
		, W(static_cast<InternalType>(Other.W))
	{
	}

	template <typename InternalType>
	template <typename OtherType>
	constexpr Vector4<InternalType>::Vector4(Vector3<OtherType> V3, OtherType InW)
		: X(static_cast<InternalType>(V3.X))
		, Y(static_cast<InternalType>(V3.Y))
		, Z(static_cast<InternalType>(V3.Z))
		, W(static_cast<InternalType>(InW))
	{
	}

	template <typename InternalType>
	constexpr Vector4<InternalType> Vector4<InternalType>::operator+(const Vector4& V) const
	{
		return Vector4(X + V.X, Y + V.Y, Z + V.Z, W + V.W);
	}

	template <typename InternalType>
	constexpr Vector4<InternalType> Vector4<InternalType>::operator-(const Vector4& V) const
	{
		return *this + (-V);
	}

	template <typename InternalType>
	constexpr Vector4<InternalType> Vector4<InternalType>::operator*(const Vector4& V) const
	{
		return Vector4(X * V.X, Y * V.Y, Z * V.Z, W * V.W);
	}

	template <typename InternalType>
	constexpr Vector4<InternalType> Vector4<InternalType>::operator/(const Vector4& V) const
	{
		return Vector4(X / V.X, Y / V.Y, Z / V.Z, W / V.W);
	}

	template <typename InternalType>
	constexpr Vector4<InternalType> Vector4<InternalType>::operator+(InternalType B) const
	{
		return Vector4(X + B, Y + B, Z + B, W + B);
	}

	template <typename InternalType>
	constexpr Vector4<InternalType> Vector4<InternalType>::operator-(InternalType B) const
	{
		return *this + (-B);
	}

	template <typename InternalType>
	constexpr Vector4<InternalType> Vector4<InternalType>::operator*(InternalType B) const
	{
		return Vector4(X * B, Y * B, Z * B, W * B);
	}

	template <typename InternalType>
	constexpr Vector4<InternalType> Vector4<InternalType>::operator/(InternalType B) const
	{
		return *this * (1 / B);
	}

	template <typename InternalType>
	constexpr Vector4<InternalType>& Vector4<InternalType>::operator+=(const Vector4& V)
	{
		*this = *this + V;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector4<InternalType>& Vector4<InternalType>::operator-=(const Vector4& V)
	{
		*this = *this - V;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector4<InternalType>& Vector4<InternalType>::operator*=(const Vector4& V)
	{
		*this = *this * V;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector4<InternalType>& Vector4<InternalType>::operator/=(const Vector4& V)
	{
		*this = *this / V;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector4<InternalType>& Vector4<InternalType>::operator+=(InternalType B)
	{
		*this = *this + B;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector4<InternalType>& Vector4<InternalType>::operator-=(InternalType B)
	{
		*this = *this - B;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector4<InternalType>& Vector4<InternalType>::operator*=(InternalType B)
	{
		*this = *this * B;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector4<InternalType>& Vector4<InternalType>::operator/=(InternalType B)
	{
		*this = *this / B;
		return *this;
	}

	template <typename InternalType>
	constexpr bool Vector4<InternalType>::operator==(const Vector4& V) const
	{
		return (X == V.X) && (Y == V.Y) && (Z == V.Z) && (W == V.W);
	}

	template <typename InternalType>
	constexpr bool Vector4<InternalType>::operator!=(const Vector4& V) const
	{
		return !(*this == V);
	}

	template <typename InternalType>
	constexpr InternalType& Vector4<InternalType>::operator[](size_t Index)
	{
		return (&X)[Index];
	}

	template <typename InternalType>
	constexpr InternalType Vector4<InternalType>::operator[](size_t Index) const
	{
		return (&X)[Index];
	}

	template <typename InternalType>
	constexpr InternalType Vector4<InternalType>::operator|(const Vector4& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z + W * V.W;
	}

	template <typename InternalType>
	constexpr Vector4<InternalType> Vector4<InternalType>::operator-() const
	{
		return Vector4(-X, -Y, -Z, -W);
	}

	template <typename InternalType>
	constexpr InternalType Vector4<InternalType>::Length() const
	{
		return sqrt(LengthSq());
	}

	template <typename InternalType>
	constexpr InternalType Vector4<InternalType>::LengthSq() const
	{
		return X * X + Y * Y + Z * Z + W * W;
	}

	template <typename InternalType>
	constexpr Vector4<InternalType>& Vector4<InternalType>::Normalize()
	{
		*this /= Length();
		return *this;
	}

	template <typename InternalType>
	constexpr Vector4<InternalType>& Vector4<InternalType>::SafeNormalize(InternalType Epsilon)
	{
		if (!IsCloseToZero(Epsilon))
			return Normalize();
		return *this;
	}

	template <typename InternalType>
	constexpr bool Vector4<InternalType>::IsCloseToZero(InternalType Epsilon) const
	{
		return Math::Abs(X) < Epsilon && Math::Abs(Y) < Epsilon && Math::Abs(Z) < Epsilon && Math::Abs(W) < Epsilon; 
	}
}
