#include "FromString.h"

namespace Hermes
{
	// FIXME: exponential notation
	std::optional<double> FromString::Double(StringView String)
	{
		const char* Current = String.data();
		const char* OnePastEnd = String.data() + String.length();

		if (Current >= OnePastEnd)
			return {};

		auto Peek = [&]() -> std::optional<char>
		{
			if (Current >= OnePastEnd)
				return {};
			return *Current;
		};
		auto Consume = [&]()
		{
			if (Current < OnePastEnd)
				++Current;
		};
		auto IsDigit = [](char Char) { return Char >= '0' && Char <= '9'; };

		bool Negative = false;
		if (Peek() == '-')
		{
			Negative = true;
			Consume();
		}

		uint64 WholePart = 0;
		while (true)
		{
			if (!Peek().has_value() || !IsDigit(Peek().value()))
				break;
			WholePart = WholePart * 10 + (Peek().value() - '0');
			Consume();
		}

		// No dot - no fractional part
		if (!Peek().has_value() || Peek().value() != '.')
		{
			return static_cast<double>(WholePart) * (Negative ? -1.0 : 1.0);
		}
		Consume();

		uint64 FractionalPart = 0;
		double FractionalPartMultiplier = 1.0;
		while (true)
		{
			if (!Peek().has_value() || !IsDigit(Peek().value()))
				break;

			FractionalPart = FractionalPart * 10 + (Peek().value() - '0');
			FractionalPartMultiplier *= 0.1;
			Consume();
		}

		return (Negative ? -1.0 : 1.0) * (static_cast<double>(WholePart) + static_cast<double>(FractionalPart) * FractionalPartMultiplier);
	}
}
