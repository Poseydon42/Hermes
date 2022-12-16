#include "JSONParser.h"

#include "Logging/Logger.h"

namespace Hermes
{
	// FIXME: this whole thing needs some Unicode support
	// FIXME: and some error messages instead of just returning empty optional

	static bool IsWhitespace(char Char)
	{
		return (Char == ' ' || Char == '\n' || Char == '\r' || Char == '\t');
	}

	static bool IsDigit(char Char)
	{
		return (Char >= '0' && Char <= '9');
	}

	std::optional<std::unique_ptr<JSONObject>> JSONParser::FromString(StringView InSource)
	{
		return JSONParser(InSource).ParseObject();
	}

	JSONParser::JSONParser(StringView InSource)
		: Source(InSource)
		, Current(InSource.begin())
	{
	}

	void JSONParser::SkipWhitespaces()
	{
		while (Peek().has_value() && IsWhitespace(Peek().value()))
			Consume();
	}

	std::optional<char> JSONParser::Peek() const
	{
		if (Current < Source.end())
			return *Current;
		return {};
	}

	void JSONParser::Consume(size_t Count)
	{
		while (Count--)
		{
			if (Current < Source.end())
				++Current;
		}
	}

	std::optional<char> JSONParser::Consume()
	{
		if (Current < Source.end())
			return *Current++;
		return {};
	}

	// FIXME: Unicode sequences
	std::optional<String> JSONParser::ConsumeAndUnescapeString()
	{
		StringStream Result;
		while (Peek().has_value() && Peek().value() != '"')
		{
			auto Char = Consume().value();

			if (Char != '\\')
			{
				Result << Char;
				continue;
			}

			if (!Peek().has_value())
				return {};

			switch (Peek().value())
			{
			case '"':
				Result << '"';
				break;
			case '\\':
				Result << '\\';
				break;
			case '/':
				Result << '/';
				break;
			case 'b':
				Result << '\b';
				break;
			case 'f':
				Result << '\f';
				break;
			case 'n':
				Result << '\n';
				break;
			case 'r':
				Result << '\r';
				break;
			case 't':
				Result << '\t';
				break;
			default:
				return {};
			}
		}

		return Result.str();
	}

	bool JSONParser::CompareStringAheadUntilWhitespace(StringView String)
	{
		auto StoredCurrent = Current;

		bool Result = true;
		for (auto Char : String)
		{
			if (Consume() != Char)
			{
				Result = false;
				break;
			}
		}

		Current = StoredCurrent;
		return Result;
	}

	std::optional<std::unique_ptr<JSONObject>> JSONParser::ParseObject()
	{
		SkipWhitespaces();

		if (!Peek().has_value() || Peek().value() != '{')
			return {};
		Consume(); // Opening brace

		JSONObject::PropertiesContainerType Properties;

		while (true)
		{
			auto NextKeyValuePair = ParseKeyValuePair();
			if (!NextKeyValuePair.has_value())
				return {};

			Properties[NextKeyValuePair.value().first] = std::move(NextKeyValuePair.value().second);

			SkipWhitespaces();
			if (!Peek().has_value() || Peek().value() != ',')
				break;
			Consume(); // Semicolon between key-value pairs
		}

		if (!Peek().has_value() || Peek().value() != '}')
			return {};
		Consume(); // Closing brace

		auto Result = std::make_unique<JSONObject>(std::move(Properties));
		return Result;
	}

	std::optional<std::pair<String, JSONValue>> JSONParser::ParseKeyValuePair()
	{
		SkipWhitespaces();

		auto MaybeName = ParseString();
		if (!MaybeName.has_value())
			return {};

		SkipWhitespaces();

		if (!Peek().has_value() || Peek().value() != ':')
			return {};
		Consume(); // Colon

		auto MaybeValue = ParseValue();
		if (!MaybeValue.has_value())
			return {};

		return std::make_pair(String(MaybeName.value()), std::move(MaybeValue.value()));
	}

	std::optional<JSONValue> JSONParser::ParseValue()
	{
		SkipWhitespaces();
		auto FirstChar = Peek();
		if (!FirstChar.has_value())
			return {};
		if (FirstChar == '"')
		{
			auto MaybeString = ParseString();
			if (!MaybeString.has_value())
				return {};
			return JSONValue(MaybeString.value());
		}
		else if (FirstChar == '-' || IsDigit(FirstChar.value()))
		{
			auto MaybeNumber = ParseNumber();
			if (!MaybeNumber.has_value())
				return {};
			return JSONValue(MaybeNumber.value());
		}
		else if (FirstChar == '[')
		{
			auto MaybeArray = ParseArray();
			if (!MaybeArray.has_value())
				return {};
			return JSONValue(std::move(MaybeArray.value()));
		}
		else if (FirstChar == '{')
		{
			auto MaybeObject = ParseObject();
			if (!MaybeObject.has_value())
				return {};
			return JSONValue(std::move(MaybeObject.value()));
		}
		else if (CompareStringAheadUntilWhitespace("true"))
		{
			Consume(4);
			return JSONValue(true);
		}
		else if (CompareStringAheadUntilWhitespace("false"))
		{
			Consume(5);
			return JSONValue(false);
		}
		else if (CompareStringAheadUntilWhitespace("null"))
		{
			Consume(4);
			return JSONValue();
		}

		return {};
	}

	// FIXME: numbers in standard/exponential form
	std::optional<double> JSONParser::ParseNumber()
	{
		SkipWhitespaces();

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

		return (Negative ? -1.0 : 1.0 ) * (static_cast<double>(WholePart) + static_cast<double>(FractionalPart) * FractionalPartMultiplier);
	}

	std::optional<std::vector<JSONValue>> JSONParser::ParseArray()
	{
		SkipWhitespaces();

		if (!Peek().has_value() || Peek().value() != '[')
			return {};
		Consume();

		std::vector<JSONValue> Result;

		while (true)
		{
			auto MaybeNextValue = ParseValue();
			if (!MaybeNextValue.has_value())
				return {};
			Result.push_back(std::move(MaybeNextValue.value()));

			if (!Peek().has_value() || Peek().value() != ',')
				break;
			Consume();
		}

		SkipWhitespaces();
		if (!Peek().has_value() || Peek().value() != ']')
			return {};
		Consume();

		return Result;
	}

	std::optional<String> JSONParser::ParseString()
	{
		SkipWhitespaces();

		if (!Peek().has_value() || Peek().value() != '"')
			return {};
		Consume();

		auto String = ConsumeAndUnescapeString();
		if (!String.has_value())
			return {};

		if (!Peek().has_value() || Peek().value() != '"')
			return {};
		Consume();

		return String;
	}
}
