#pragma once

#include <optional>

#include "Core/Core.h"

namespace Hermes
{
	class FromString
	{
	public:
		/*
		 * Parses a string representation of a real number
		 */
		static std::optional<double> Double(StringView String);
	};
}
