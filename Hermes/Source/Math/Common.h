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
			return static_cast<T>(sin(Value));
		}

		template<typename T>
		float Cos(T Value)
		{
			return static_cast<T>(cos(Value));
		}

		template<typename T>
		float Tan(T Value)
		{
			return static_cast<T>(tan(Value));
		}

		template<typename T>
		float Cotan(T Value)
		{
			return static_cast<T>(tan(HalfPi - Value));
		}

		template<typename T>
		T Radians(T Degrees)
		{
			return static_cast<T>(Pi) / static_cast<T>(180) * Degrees;
		}

		template<typename T>
		T Degrees(T Radians)
		{
			return static_cast<T>(180) / static_cast<T>(Pi) * Radians;
		}

		/*
		 * Fast floor(log2) calculation for integer types
		 * Returns static_cast<T>(-1) if the number is zero
		 */
		template<typename T>
		uint32 FloorLog2(T Value)
		{
			uint32 BitNumber = sizeof(Value) * 8;
			uint32 Index = BitNumber;
			if (!Value)
				return static_cast<uint32>(-1);
			while (Index--)
			{
				if (Value & (static_cast<uint64>(0x01) << (BitNumber - 1)))
					return Index;
				Value <<= 1;
			}
			HERMES_ASSERT(false);
			return 0;
		}
	}
}
