#pragma once

#include "Core/Core.h"
#include "Math/Common.h"
#include <cmath>

namespace Hermes
{
	template<typename InternalType>
	struct Vector2
	{
		InternalType X;
		InternalType Y;

		constexpr Vector2(InternalType V = 0.0f);

		constexpr Vector2(InternalType NX, InternalType NY);

		template<typename OtherType>
		explicit constexpr Vector2(Vector2<OtherType> Other);

		/**
		 * Bitwise operators
		 */
		constexpr Vector2 operator+(const Vector2& V) const;
		constexpr Vector2 operator-(const Vector2& V) const;
		constexpr Vector2 operator*(const Vector2& V) const;
		constexpr Vector2 operator/(const Vector2& V) const;
		constexpr Vector2 operator+(InternalType B) const;
		constexpr Vector2 operator-(InternalType B) const;
		constexpr Vector2 operator*(InternalType B) const;
		constexpr Vector2 operator/(InternalType B) const;
		constexpr Vector2& operator+=(const Vector2& V);
		constexpr Vector2& operator-=(const Vector2& V);
		constexpr Vector2& operator*=(const Vector2& V);
		constexpr Vector2& operator/=(const Vector2& V);
		constexpr Vector2& operator+=(InternalType B);
		constexpr Vector2& operator-=(InternalType B);
		constexpr Vector2& operator*=(InternalType B);
		constexpr Vector2& operator/=(InternalType B);

		constexpr bool operator==(const Vector2& V) const;
		constexpr bool operator!=(const Vector2& V) const;

		constexpr InternalType& operator[](size_t Index);
		constexpr InternalType operator[](size_t Index) const;
		
		/**
		 * Dot product
		 */
		constexpr float operator|(const Vector2& V) const;

		/**
		 * Negate(flip) the vector
		 */
		constexpr Vector2 operator-() const;

		constexpr InternalType Length() const;

		constexpr InternalType LengthSq() const;

		/**
		 * Normalizes a vector and returns reference to itself
		 */
		constexpr Vector2& Normalize();

		/*
		 * Normalizes a vector only if at least one of its components is greater than zero
		 * with a tolerance of Epsilon and returns reference to itself
		 */
		constexpr Vector2& SafeNormalize(InternalType Epsilon = InternalType(FLT_EPSILON));

		/*
		 * Returns true if all components of vector are close to zero with a tolerance of Epsilon
		 */
		constexpr bool IsCloseToZero(InternalType Epsilon = InternalType(FLT_EPSILON)) const;
	};

	/**
	 * Aliases for vectors of basic types
	 */
	using Vec2 = Vector2<float>;
	using Vec2i = Vector2<int32>;
	using Vec2ui = Vector2<uint32>;
	using Vec2l = Vector2<int64>;
	using Vec2d = Vector2<double>;

	template <typename InternalType>
	constexpr Vector2<InternalType>::Vector2(InternalType V) : X(V), Y(V) {}

	template <typename InternalType>
	constexpr Vector2<InternalType>::Vector2(InternalType NX, InternalType NY) : X(NX), Y(NY) {}

	template <typename InternalType>
	template <typename OtherType>
	constexpr Vector2<InternalType>::Vector2(Vector2<OtherType> Other)
		: X(static_cast<InternalType>(Other.X))
		, Y(static_cast<InternalType>(Other.Y))
	{
	}

	template <typename InternalType>
	constexpr Vector2<InternalType> Vector2<InternalType>::operator+(const Vector2& V) const
	{
		return Vector2(X + V.X, Y + V.Y);
	}

	template <typename InternalType>
	constexpr Vector2<InternalType> Vector2<InternalType>::operator-(const Vector2& V) const
	{
		return *this + (-V);
	}

	template <typename InternalType>
	constexpr Vector2<InternalType> Vector2<InternalType>::operator*(const Vector2& V) const
	{
		return Vector2(X * V.X, Y * V.Y);
	}

	template <typename InternalType>
	constexpr Vector2<InternalType> Vector2<InternalType>::operator/(const Vector2& V) const
	{
		return Vector2(X / V.X, Y / V.Y);
	}

	template <typename InternalType>
	constexpr Vector2<InternalType> Vector2<InternalType>::operator+(InternalType B) const
	{
		return Vector2(X + B, Y + B);
	}

	template <typename InternalType>
	constexpr Vector2<InternalType> Vector2<InternalType>::operator-(InternalType B) const
	{
		return *this + (-B);
	}

	template <typename InternalType>
	constexpr Vector2<InternalType> Vector2<InternalType>::operator*(InternalType B) const
	{
		return Vector2(X * B, Y * B);
	}

	template <typename InternalType>
	constexpr Vector2<InternalType> Vector2<InternalType>::operator/(InternalType B) const
	{
		return *this * (1 / B);
	}

	template <typename InternalType>
	constexpr Vector2<InternalType>& Vector2<InternalType>::operator+=(const Vector2& V)
	{
		*this = *this + V;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector2<InternalType>& Vector2<InternalType>::operator-=(const Vector2& V)
	{
		*this = *this - V;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector2<InternalType>& Vector2<InternalType>::operator*=(const Vector2& V)
	{
		*this = *this * V;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector2<InternalType>& Vector2<InternalType>::operator/=(const Vector2& V)
	{
		*this = *this / V;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector2<InternalType>& Vector2<InternalType>::operator+=(InternalType B)
	{
		*this = *this + B;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector2<InternalType>& Vector2<InternalType>::operator-=(InternalType B)
	{
		*this = *this - B;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector2<InternalType>& Vector2<InternalType>::operator*=(InternalType B)
	{
		*this = *this * B;
		return *this;
	}

	template <typename InternalType>
	constexpr Vector2<InternalType>& Vector2<InternalType>::operator/=(InternalType B)
	{
		*this = *this / B;
		return *this;
	}

	template <typename InternalType>
	constexpr bool Vector2<InternalType>::operator==(const Vector2& V) const
	{
		return (X == V.X) && (Y == V.Y);
	}

	template <typename InternalType>
	constexpr bool Vector2<InternalType>::operator!=(const Vector2& V) const
	{
		return !(*this == V);
	}

	template <typename InternalType>
	constexpr InternalType& Vector2<InternalType>::operator[](size_t Index)
	{
		return (&X)[Index];
	}

	template <typename InternalType>
	constexpr InternalType Vector2<InternalType>::operator[](size_t Index) const
	{
		return (&X)[Index];
	}

	template <typename InternalType>
	constexpr float Vector2<InternalType>::operator|(const Vector2& V) const
	{
		return X * V.X + Y * V.Y;
	}

	template <typename InternalType>
	constexpr Vector2<InternalType> Vector2<InternalType>::operator-() const
	{
		return Vector2(-X, -Y);
	}

	template <typename InternalType>
	constexpr InternalType Vector2<InternalType>::Length() const
	{
		return sqrt(LengthSq());
	}

	template <typename InternalType>
	constexpr InternalType Vector2<InternalType>::LengthSq() const
	{
		return X * X + Y * Y;
	}

	template <typename InternalType>
	constexpr Vector2<InternalType>& Vector2<InternalType>::Normalize()
	{
		*this /= Length();
		return *this;
	}

	template <typename InternalType>
	constexpr Vector2<InternalType>& Vector2<InternalType>::SafeNormalize(InternalType Epsilon)
	{
		if (!IsCloseToZero(Epsilon))
			return Normalize();
		return *this;
	}

	template <typename InternalType>
	constexpr bool Vector2<InternalType>::IsCloseToZero(InternalType Epsilon) const
	{
		return Math::Abs(X) < Epsilon && Math::Abs(Y) < Epsilon;
	}
}
