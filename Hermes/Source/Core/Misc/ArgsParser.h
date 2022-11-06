#pragma once

#include <optional>
#include <vector>

#include "Core/Core.h"

namespace Hermes
{
	class HERMES_API ArgsParser
	{
	public:
		/*
		 * Adds boolean option argument. Value is set to true when --Name or -ShortName is present
		 * in the argument list or if --Name= or -ShortName= is followed by 'true' or '1'(matching the case)
		 */
		void AddOption(const String& Name, std::optional<char> ShortName, bool* Value);

		/*
		 * Adds string option argument. When --Name=... or -ShortName=... is met in the argument list,
		 * the part after equality sign is assigned to Value
		 */
		void AddOption(const String& Name, std::optional<char> ShortName, String* Value);

		/*
		 * Adds positional argument. Values to the positional arguments are assigned in the order of
		 * AddPositionalArgument() calls. If there is not enough arguments passed then the untouched
		 * pointers will be assigned to their default value(empty string)
		 * IsRequired sets whether Parse() will result in failure or success if user did not provide
		 * argument
		 */
		void AddPositional(bool IsRequired, String* Value);

		/*
		 * Parses passed arguments and assigns values to previously passed pointers correspondingly.
		 * All pointers that were passed to Add* functions must be valid until this function returns
		 */
		bool Parse(int ArgCount, const char** ArgValues) const;

	private:
		enum class ValueType
		{
			Bool,
			String
		};

		struct OptionalArgument
		{
			String Name;
			std::optional<char> ShortName;
			ValueType Type;
			void* ValuePtr;
		};

		std::vector<OptionalArgument> OptionalArguments;
		std::vector<std::pair<bool, String*>> PositionalArguments; // pair<IsRequired, Ptr>
	};
}
