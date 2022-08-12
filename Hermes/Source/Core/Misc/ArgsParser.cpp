#include "ArgsParser.h"

#include "StringUtils.h"

namespace Hermes
{
	void ArgsParser::AddOption(const ANSIString& Name, std::optional<char> ShortName, bool* Value)
	{
		OptionalArguments.emplace_back(Name, ShortName, ValueType::Bool, Value);
	}

	void ArgsParser::AddOption(const ANSIString& Name, std::optional<char> ShortName, String* Value)
	{
		OptionalArguments.emplace_back(Name, ShortName, ValueType::String, Value);
	}

	void ArgsParser::AddPositional(bool IsRequired, String* Value)
	{
		PositionalArguments.emplace_back(IsRequired, Value);
	}

	bool ArgsParser::Parse(int ArgCount, const char** ArgValues) const
	{
		bool ReturnValue = true;

		const char** NextArgument = ArgValues;
		const char** LastArgument = ArgValues + ArgCount;
		size_t NextPositionalArgumentIndex = 0;

		for (; NextArgument != LastArgument; NextArgument++)
		{
			const char* Argument = *NextArgument;
			if (!Argument)
				continue;

			// If argument starts with dash it is considered optional argument
			if (*Argument == '-')
			{
				bool FoundCorrespondingOption = false;

				for (const auto& Option : OptionalArguments)
				{
					bool AreNamesEqual = false;
					// Check if first two characters are --, then check if following characters are equal to the current option
					// name, and lastly check if it is either end of the current argument or the next character after its name is =
					if (Argument[0] == '-' && Argument[1] == '-' &&
						strncmp(Argument + 2, Option.Name.c_str(), Option.Name.length()) == 0 &&
						(Argument[2 + Option.Name.length()] == '=' || Argument[2 + Option.Name.length()] == 0))
					{
						Argument += 2 + Option.Name.length();
						AreNamesEqual = true;
					}
					// Check if first character is -, then check if the next character is equal to the short name(if available)
					// and lastly check if the following character is null terminator or =
					if (Argument[0] == '-' && Option.ShortName.has_value() &&
						Argument[1] == Option.ShortName.value() &&
						(Argument[2] == 0 || Argument[2] == '='))
					{
						Argument += 2; // Remove - symbol and the short name
						AreNamesEqual = true;
					}

					if (!AreNamesEqual)
						continue;

					FoundCorrespondingOption = true;

					// If there is no value after the argument(no = character)
					if (*Argument == 0)
					{
						switch (Option.Type)
						{
						case ValueType::Bool:
							// In case of bool set value to true in any case as the value indicates presence
							// of the argument
							*static_cast<bool*>(Option.ValuePtr) = true;
							break;
						default:
							ReturnValue = false;
							break;
						}
					}
					else if (*Argument == '=')
					{
						Argument++;
						auto ArgumentValue = ANSIString(Argument);
						switch (Option.Type)
						{
						case ValueType::Bool:
							// Set to true only if value is 'true' or '1'
							if (ArgumentValue == "true" || ArgumentValue == "1")
								*static_cast<bool*>(Option.ValuePtr) = true;
							break;
						case ValueType::String:
							*static_cast<String*>(Option.ValuePtr) = StringUtils::ANSIToString(ArgumentValue);
							break;
						}
					}
					else
					{
						ReturnValue = false; // Invalid syntax
					}
				}

				if (!FoundCorrespondingOption)
				{
					// Set return value to false because user provided optional value that
					// application does not expect
					ReturnValue = false;
				}
			}
			else
			{
				if (NextPositionalArgumentIndex >= PositionalArguments.size())
				{
					// User passed too many positional arguments
					ReturnValue = false;
					continue; // But there still might be some optional arguments that we should parse
				}

				*PositionalArguments[NextPositionalArgumentIndex++].second = StringUtils::ANSIToString(Argument);
			}
		}
		
		// And clear untouched positional argument value pointers
		for (; NextPositionalArgumentIndex < PositionalArguments.size(); NextPositionalArgumentIndex++)
		{
			*PositionalArguments[NextPositionalArgumentIndex].second = String {};
			if (PositionalArguments[NextPositionalArgumentIndex].first) // And if this argument is required then return failure
			{
				ReturnValue = false;
			}
		}

		return ReturnValue;
	}
}
