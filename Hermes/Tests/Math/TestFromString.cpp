#include <gtest/gtest.h>

#include "Math/FromString.h"

using namespace Hermes;

TEST(TestFromString, Double)
{
	auto Result = FromString::Double("3.7");

	ASSERT_TRUE(Result.has_value());
	EXPECT_NEAR(Result.value(), 3.7, 0.00001);
}

TEST(TestFromString, DoubleWithExponent)
{
	auto Result1 = FromString::Double("-3.7e-2");

	ASSERT_TRUE(Result1.has_value());
	EXPECT_NEAR(Result1.value(), -3.7e-2, 0.00001);

	auto Result2 = FromString::Double("3.7E12");

	ASSERT_TRUE(Result2.has_value());
	EXPECT_NEAR(Result2.value(), 3.7E12, 0.00001);
}

TEST(TestFromString, IntegerAsDouble)
{
	auto Result = FromString::Double("42");

	ASSERT_TRUE(Result.has_value());
	EXPECT_NEAR(Result.value(), 42.0, 0.00001);
}
