#pragma once

#include "Core/Core.h"
#include "Math/Vector2.h"

namespace Hermes
{
	/*
	 * Represents a rectangle in 2D space via its minimum and maximum X and Y coordinates.
	 *
	 * Assumes that the Y axis grows downwards.
	 */
	template<typename ComponentType>
	struct Rectangle2D
	{
		Vector2<ComponentType> Min;
		Vector2<ComponentType> Max;

		static Rectangle2D FromCoordinatesAndWidth(Vector2<ComponentType> Origin, ComponentType Width, ComponentType Height);

		ComponentType Top() const;
		ComponentType& Top();

		ComponentType Bottom() const;
		ComponentType& Bottom();

		ComponentType Left() const;
		ComponentType& Left();

		ComponentType Right() const;
		ComponentType& Right();

		ComponentType Width() const;
		ComponentType Height() const;

		Vector2<ComponentType> Dimensions() const;

		Rectangle2D Intersect(const Rectangle2D& Other) const;

		template<typename VectorComponentType>
		bool Contains(Vector2<VectorComponentType> Point) const;

		bool operator==(const Rectangle2D& Other) const;
		bool operator!=(const Rectangle2D& Other) const;
	};

	template<typename ComponentType>
	Rectangle2D<ComponentType> Rectangle2D<ComponentType>::FromCoordinatesAndWidth(Vector2<ComponentType> Origin, ComponentType Width, ComponentType Height)
	{
		return { Origin, { Origin.X + Width, Origin.Y + Height } };
	}

	template<typename ComponentType>
	ComponentType Rectangle2D<ComponentType>::Top() const
	{
		return Min.Y;
	}

	template<typename ComponentType>
	ComponentType& Rectangle2D<ComponentType>::Top()
	{
		return Min.Y;
	}

	template<typename ComponentType>
	ComponentType Rectangle2D<ComponentType>::Bottom() const
	{
		return Max.Y;
	}

	template<typename ComponentType>
	ComponentType& Rectangle2D<ComponentType>::Bottom()
	{
		return Max.Y;
	}

	template<typename ComponentType>
	ComponentType Rectangle2D<ComponentType>::Left() const
	{
		return Min.X;
	}

	template<typename ComponentType>
	ComponentType& Rectangle2D<ComponentType>::Left()
	{
		return Min.X;
	}

	template<typename ComponentType>
	ComponentType Rectangle2D<ComponentType>::Right() const
	{
		return Max.X;
	}

	template<typename ComponentType>
	ComponentType& Rectangle2D<ComponentType>::Right()
	{
		return Max.X;
	}

	template<typename ComponentType>
	ComponentType Rectangle2D<ComponentType>::Width() const
	{
		return Max.X - Min.X;
	}

	template<typename ComponentType>
	ComponentType Rectangle2D<ComponentType>::Height() const
	{
		return Max.Y - Min.Y;
	}

	template<typename ComponentType>
	Vector2<ComponentType> Rectangle2D<ComponentType>::Dimensions() const
	{
		return { Width(), Height() };
	}

	template<typename ComponentType>
	Rectangle2D<ComponentType> Rectangle2D<ComponentType>::Intersect(const Rectangle2D& Other) const
	{
		auto Result =  Rectangle2D {
			.Min = { Math::Max(Min.X, Other.Min.X), Math::Max(Min.Y, Other.Min.Y) },
			.Max = { Math::Min(Max.X, Other.Max.X), Math::Min(Max.Y, Other.Max.Y) }
		};

		if (Result.Max.X < Result.Min.X || Result.Max.Y < Result.Min.Y)
			Result = { {}, {} };

		return Result;
	}

	template<typename ComponentType>
	template<typename VectorComponentType>
	bool Rectangle2D<ComponentType>::Contains(Vector2<VectorComponentType> Point) const
	{
		return Min.X <= Point.X && Max.X > Point.X && Min.Y <= Point.Y && Max.Y > Point.Y;
	}

	template<typename ComponentType>
	bool Rectangle2D<ComponentType>::operator==(const Rectangle2D& Other) const
	{
		return Min == Other.Min && Max == Other.Max;
	}

	template<typename ComponentType>
	bool Rectangle2D<ComponentType>::operator!=(const Rectangle2D& Other) const
	{
		return !(*this == Other);
	}

	using Rect2D = Rectangle2D<float>;
	using Rect2Di = Rectangle2D<int32>;
	using Rect2Dui = Rectangle2D<uint32>;
}
