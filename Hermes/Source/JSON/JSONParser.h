#pragma once

#include <memory>
#include <optional>

#include "Core/Core.h"
#include "JSON/JSONObject.h"

namespace Hermes
{
	class HERMES_API JSONParser
	{
	public:
		static std::optional<std::unique_ptr<JSONObject>> FromString(StringView InSource);

	private:
		StringView Source;

		StringView::const_iterator Current;

		JSONParser(StringView InSource);

		void SkipWhitespaces();

		std::optional<char> Peek() const;

		std::optional<char> Consume();

		void Consume(size_t Count);

		std::optional<String> ConsumeAndUnescapeString();

		bool CompareStringAheadUntilWhitespace(StringView String);

		std::optional<std::unique_ptr<JSONObject>> ParseObject();

		std::optional<std::pair<String, JSONValue>> ParseKeyValuePair();

		std::optional<JSONValue> ParseValue();

		std::optional<double> ParseNumber();

		std::optional<std::vector<JSONValue>> ParseArray();

		std::optional<String> ParseString();
	};
}
