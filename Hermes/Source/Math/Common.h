#pragma once

#include "Core/Core.h"

namespace Hermes
{
	namespace Math
	{
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
	}
}
