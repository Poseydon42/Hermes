#include <gtest/gtest.h>

#include "JSON/JSONObject.h"
#include "JSON/JSONValue.h"

using namespace Hermes;

TEST(TestJSONValue, NullValue)
{
	JSONValue Value;
	EXPECT_TRUE(Value.Is(JSONValueType::Null));
}

TEST(TestJSONValue, StringValue)
{
	JSONValue Value(String("test_value"));
	EXPECT_TRUE(Value.Is(JSONValueType::String));
	EXPECT_EQ(Value.AsString(), "test_value");
}

TEST(TestJSONValue, NumberValue)
{
	JSONValue Value(4.2);
	EXPECT_TRUE(Value.Is(JSONValueType::Number));
	EXPECT_EQ(Value.AsNumber(), 4.2);
	EXPECT_EQ(Value.AsInteger(), 4);
}

TEST(TestJSONValue, BoolValue)
{
	JSONValue Value(true);
	EXPECT_TRUE(Value.Is(JSONValueType::Bool));
	EXPECT_TRUE(Value.AsBool());
}

TEST(TestJSONValue, ObjectValue)
{
	auto Object = std::make_unique<JSONObject>();
	const auto* ObjectPtr = Object.get();

	JSONValue Value(std::move(Object));
	EXPECT_TRUE(Value.Is(JSONValueType::Object));
	EXPECT_EQ(&Value.AsObject(), ObjectPtr);
}

TEST(TestJSONValue, ArrayValue)
{
	auto InnerValue = JSONValue(4.2);
	
	JSONValue Value({ std::move(InnerValue) });
	EXPECT_TRUE(Value.Is(JSONValueType::Array));
	EXPECT_TRUE(Value.AsArray()[0].Is(JSONValueType::Number));
	EXPECT_EQ(Value.AsArray()[0].AsNumber(), 4.2);
}
