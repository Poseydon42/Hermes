#include <gtest/gtest.h>

#include "Math/FromString.h"

using namespace Hermes;

TEST(TestFromString, Double)
{
	auto Result = FromString::Double("3.7");

	ASSERT_TRUE(Result.has_value());
	EXPECT_NEAR(Result.value(), 3.7, 0.00001);
}

TEST(TestFromString, IntegerAsDouble)
{
	auto Result = FromString::Double("42");

	ASSERT_TRUE(Result.has_value());
	EXPECT_NEAR(Result.value(), 42.0, 0.00001);
}
