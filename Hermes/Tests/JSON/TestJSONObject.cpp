#include <gtest/gtest.h>

#include "JSON/JSONObject.h"

using namespace Hermes;

TEST(TestJSONObject, EmptyObject)
{
	JSONObject Object;

	EXPECT_EQ(Object.GetNumberOfProperties(), 0);
	EXPECT_EQ(std::distance(Object.begin(), Object.end()), 0);
}

TEST(TestJSONObject, ObjectWithProperties)
{
	std::unordered_map<String, JSONValue> Values;
	Values["foo"] = JSONValue(String("bar"));
	Values["baz"] = JSONValue(std::make_unique<JSONObject>());

	JSONObject Object(std::move(Values));

	EXPECT_EQ(Object.GetNumberOfProperties(), 2);
	EXPECT_EQ(std::distance(Object.begin(), Object.end()), 2);

	ASSERT_TRUE(Object.Contains("foo"));
	EXPECT_TRUE(Object.Get("foo").Is(JSONValueType::String));

	ASSERT_TRUE(Object.Contains("baz"));
	EXPECT_TRUE(Object["baz"].Is(JSONValueType::Object));
}
