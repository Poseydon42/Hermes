#include <gtest/gtest.h>

#include "JSON/JSONParser.h"

using namespace Hermes;

TEST(TestJSONParser, SimpleObject)
{
	StringView JSON = R"({ "foo": "bar" })";

	auto MaybeRootObject = JSONParser::FromString(JSON);

	ASSERT_TRUE(MaybeRootObject.has_value());

	auto RootObject = std::move(MaybeRootObject.value());
	ASSERT_TRUE(RootObject->Contains("foo"));
	const auto& Foo = RootObject->Get("foo");
	EXPECT_TRUE(Foo.Is(JSONValueType::String));
	EXPECT_EQ(Foo.AsString(), "bar");
}

TEST(TestJSONParser, NumericValue)
{
	StringView JSON = R"({ "foo": -4.2 })";

	auto MaybeRootObject = JSONParser::FromString(JSON);

	ASSERT_TRUE(MaybeRootObject.has_value());

	auto RootObject = std::move(MaybeRootObject.value());
	ASSERT_TRUE(RootObject->Contains("foo"));
	const auto& Foo = RootObject->Get("foo");
	EXPECT_TRUE(Foo.Is(JSONValueType::Number));
	EXPECT_NEAR(Foo.AsNumber(), -4.2, 0.0001);
}

TEST(TestJSONParser, TrueValue)
{
	StringView JSON = R"({ "foo": true })";

	auto MaybeRootObject = JSONParser::FromString(JSON);

	ASSERT_TRUE(MaybeRootObject.has_value());

	auto RootObject = std::move(MaybeRootObject.value());
	ASSERT_TRUE(RootObject->Contains("foo"));
	const auto& Foo = RootObject->Get("foo");
	EXPECT_TRUE(Foo.Is(JSONValueType::Bool));
	EXPECT_EQ(Foo.AsBool(), true);
}

TEST(TestJSONParser, FalseValue)
{
	StringView JSON = R"({ "foo": false })";

	auto MaybeRootObject = JSONParser::FromString(JSON);

	ASSERT_TRUE(MaybeRootObject.has_value());

	auto RootObject = std::move(MaybeRootObject.value());
	ASSERT_TRUE(RootObject->Contains("foo"));
	const auto& Foo = RootObject->Get("foo");
	EXPECT_TRUE(Foo.Is(JSONValueType::Bool));
	EXPECT_EQ(Foo.AsBool(), false);
}

TEST(TestJSONParser, NullValue)
{
	StringView JSON = R"({ "foo": null })";

	auto MaybeRootObject = JSONParser::FromString(JSON);

	ASSERT_TRUE(MaybeRootObject.has_value());

	auto RootObject = std::move(MaybeRootObject.value());
	ASSERT_TRUE(RootObject->Contains("foo"));
	const auto& Foo = RootObject->Get("foo");
	EXPECT_TRUE(Foo.Is(JSONValueType::Null));
}

TEST(TestJSONParser, MultipleValues)
{
	StringView JSON = R"({ "foo": "bar", "fiz": "buz" })";

	auto MaybeRootObject = JSONParser::FromString(JSON);

	ASSERT_TRUE(MaybeRootObject.has_value());

	auto RootObject = std::move(MaybeRootObject.value());
	EXPECT_TRUE(RootObject->Contains("foo"));
	EXPECT_TRUE(RootObject->Contains("fiz"));
}

TEST(TestJSONParser, ArrayValue)
{
	StringView JSON = R"({ "foo": [ 1, 3 ] })";

	auto MaybeRootObject = JSONParser::FromString(JSON);

	ASSERT_TRUE(MaybeRootObject.has_value());

	auto RootObject = std::move(MaybeRootObject.value());
	ASSERT_TRUE(RootObject->Contains("foo"));

	const auto& Foo = RootObject->Get("foo");
	EXPECT_TRUE(Foo.Is(JSONValueType::Array));
	ASSERT_EQ(Foo.AsArray().size(), 2);
	EXPECT_NEAR(Foo.AsArray()[0].AsNumber(), 1.0, 0.0001);
	EXPECT_NEAR(Foo.AsArray()[1].AsNumber(), 3.0, 0.0001);
}

TEST(TestJSONParser, ObjectValue)
{
	StringView JSON = R"({ "foo": { "bar": "baz" } })";

	auto MaybeRootObject = JSONParser::FromString(JSON);

	ASSERT_TRUE(MaybeRootObject.has_value());

	auto RootObject = std::move(MaybeRootObject.value());
	ASSERT_TRUE(RootObject->Contains("foo"));

	const auto& Foo = RootObject->Get("foo");
	ASSERT_TRUE(Foo.Is(JSONValueType::Object));
	ASSERT_TRUE(Foo.AsObject().Contains("bar"));
	EXPECT_TRUE(Foo.AsObject().Get("bar").Is(JSONValueType::String));
	EXPECT_EQ(Foo.AsObject().Get("bar").AsString(), "baz");
}

TEST(TestJSONParser, FullTest)
{
	StringView JSON = R"(
	{
		"foo": "bar",
		"num": 3.2,
		"bool": true,
		"object": {
			"baz": "foo"
		},
		"array": [
			1, 4, 7
		],
		"null": null
	}
	)";

	auto MaybeRootObject = JSONParser::FromString(JSON);

	ASSERT_TRUE(MaybeRootObject.has_value());

	auto RootObject = std::move(MaybeRootObject.value());

	EXPECT_EQ(RootObject->GetNumberOfProperties(), 6);

	ASSERT_TRUE(RootObject->Contains("foo"));
	EXPECT_TRUE(RootObject->Get("foo").Is(JSONValueType::String));
	EXPECT_EQ(RootObject->Get("foo").AsString(), "bar");

	ASSERT_TRUE(RootObject->Contains("num"));
	EXPECT_TRUE(RootObject->Get("num").Is(JSONValueType::Number));
	EXPECT_NEAR(RootObject->Get("num").AsNumber(), 3.2, 0.0001);

	ASSERT_TRUE(RootObject->Contains("bool"));
	EXPECT_TRUE(RootObject->Get("bool").Is(JSONValueType::Bool));
	EXPECT_EQ(RootObject->Get("bool").AsBool(), true);

	ASSERT_TRUE(RootObject->Contains("object"));
	EXPECT_TRUE(RootObject->Get("object").Is(JSONValueType::Object));
	EXPECT_EQ(RootObject->Get("object").AsObject().GetNumberOfProperties(), 1);
	EXPECT_TRUE(RootObject->Get("object").AsObject().Contains("baz"));
	EXPECT_TRUE(RootObject->Get("object").AsObject().Get("baz").Is(JSONValueType::String));
	EXPECT_EQ(RootObject->Get("object").AsObject().Get("baz").AsString(), "foo");

	ASSERT_TRUE(RootObject->Contains("array"));
	EXPECT_TRUE(RootObject->Get("array").Is(JSONValueType::Array));
	EXPECT_TRUE(RootObject->Get("array").AsArray()[0].Is(JSONValueType::Number));
	EXPECT_NEAR(RootObject->Get("array").AsArray()[1].AsNumber(), 4.0, 0.0001);
}
