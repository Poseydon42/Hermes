#include "JSONValue.h"

#include "JSON/JSONObject.h"

namespace Hermes
{
	// These have to be in .cpp file because otherwise it fails to compile due to circular #include dependency
	JSONValue::JSONValue() = default;
	JSONValue::~JSONValue() = default;
	JSONValue::JSONValue(JSONValue&&) = default;
	JSONValue& JSONValue::operator=(JSONValue&&) = default;

	JSONValue::JSONValue(std::string Value)
		: Type(JSONValueType::String)
		, StringValue(std::move(Value))
	{
	}

	JSONValue::JSONValue(double Value)
		: Type(JSONValueType::Number)
		, NumberValue(Value)
	{
	}

	JSONValue::JSONValue(bool Value)
		: Type(JSONValueType::Bool)
		, BoolValue(Value)
	{
	}

	JSONValue::JSONValue(std::unique_ptr<JSONObject> Value)
		: Type(JSONValueType::Object)
		, ObjectValue(std::move(Value))
	{
	}

	JSONValue::JSONValue(std::vector<JSONValue> Value)
		: Type(JSONValueType::Array)
		, ArrayValue(std::move(Value))
	{
	}

	JSONValueType JSONValue::GetType() const
	{
		return Type;
	}

	bool JSONValue::Is(JSONValueType OtherType) const
	{
		return Type == OtherType;
	}

	StringView JSONValue::AsString() const
	{
		return StringValue;
	}

	double JSONValue::AsNumber() const
	{
		return NumberValue;
	}

	uint64 JSONValue::AsInteger() const
	{
		return static_cast<uint64>(NumberValue);
	}

	bool JSONValue::AsBool() const
	{
		return BoolValue;
	}

	const JSONObject& JSONValue::AsObject() const
	{
		return *ObjectValue;
	}

	std::span<const JSONValue> JSONValue::AsArray() const
	{
		return ArrayValue;
	}
}
