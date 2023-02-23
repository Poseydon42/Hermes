#pragma once

#include <memory>
#include <span>
#include <vector>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	class JSONObject;

	enum class JSONValueType
	{
		String,
		Number,
		Bool,
		Object,
		Array,
		Null
	};

	class HERMES_API JSONValue
	{
		MAKE_NON_COPYABLE(JSONValue)

	public:
		JSONValue();

		~JSONValue();
		JSONValue(JSONValue&&);
		JSONValue& operator=(JSONValue&&);

		explicit JSONValue(std::string Value);

		explicit JSONValue(double Value);

		explicit JSONValue(bool Value);

		explicit JSONValue(std::unique_ptr<JSONObject> Value);

		explicit JSONValue(std::vector<JSONValue> Value);

		JSONValueType GetType() const;

		bool Is(JSONValueType OtherType) const;

		StringView AsString() const;

		double AsNumber() const;

		int64 AsInteger() const;

		bool AsBool() const;

		const JSONObject& AsObject() const;

		std::span<const JSONValue> AsArray() const;

	private:
		JSONValueType Type = JSONValueType::Null;

		String StringValue;
		double NumberValue = 0.0;
		bool BoolValue = false;
		std::unique_ptr<JSONObject> ObjectValue;
		std::vector<JSONValue> ArrayValue;
	};
}
