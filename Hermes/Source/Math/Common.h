#pragma once

#include "Core/Core.h"

namespace Hermes
{
	namespace Math
	{
		constexpr float Pi = 3.14159265359f;
		constexpr float HalfPi = Pi / 2.0f;

		template<typename T>
		T Min(T A, T B)
		{
			return (A < B ? A : B);
		}

		template<typename T>
		T Max(T A, T B)
		{
			return (A > B ? A : B);
		}

		template<typename T>
		T Abs(T Value)
		{
			return (Value >= 0 ? Value : -Value);
		}

		template<typename T>
		T Clamp(T MinValue, T MaxValue, T Value)
		{
			return Max(MinValue, Min(MaxValue, Value));
		}

		template<typename T>
		float Sin(T Value)
		{
			return sin(Value);
		}

		template<typename T>
		float Cos(T Value)
		{
			return cos(Value);
		}

		template<typename T>
		float Tan(T Value)
		{
			return tan(Value);
		}

		template<typename T>
		float Cotan(T Value)
		{
			return tan(HalfPi - Value);
		}
	}
}
