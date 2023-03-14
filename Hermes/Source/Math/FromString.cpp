#include "FromString.h"

#include "Math/Common.h"

namespace Hermes
{
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
		bool HasFractionalPart = false;
		if (Peek().has_value() && Peek().value() == '.')
		{
			HasFractionalPart = true;
			Consume();
		}

		uint64 FractionalPart = 0;
		double FractionalPartMultiplier = 1.0;
		while (HasFractionalPart)
		{
			if (!Peek().has_value() || !IsDigit(Peek().value()))
				break;

			FractionalPart = FractionalPart * 10 + (Peek().value() - '0');
			FractionalPartMultiplier *= 0.1;
			Consume();
		}

		// Exponent
		double Exponent = 0.0;
		if (Peek().has_value() && (Peek().value() == 'e' || Peek().value() == 'E'))
		{
			Consume();

			bool ExponentIsPositive = true;
			if (Peek().has_value() && Peek().value() == '+')
			{
				Consume();
			}
			if (Peek().has_value() && Peek().value() == '-')
			{
				ExponentIsPositive = false;
				Consume();
			}

			uint64 RawExponentValue = 0;
			while (Peek().has_value() && IsDigit(Peek().value()))
			{
				auto Digit = Peek().value() - '0';
				Consume();

				RawExponentValue = RawExponentValue * 10 + Digit;
			}

			Exponent = (ExponentIsPositive ? 1.0 : -1.0) * static_cast<double>(RawExponentValue);
		}

		double RawValue = static_cast<double>(WholePart) + static_cast<double>(FractionalPart) * FractionalPartMultiplier;
		double Result = (Negative ? -1.0 : 1.0) * RawValue * Math::Pow(10.0, Exponent);

		return Result;
	}
}
