#include <gtest/gtest.h>

#include "Core/Misc/Version.h"

using namespace Hermes;

TEST(TestVersion, Constructor)
{
	Version Value(1, 3, 7);
	EXPECT_EQ(Value.Major, 1);
	EXPECT_EQ(Value.Minor, 3);
	EXPECT_EQ(Value.Patch, 7);
}

TEST(TestVersion, Equality)
{
	Version Value1(1, 4, 2);
	Version Value2(1, 4, 2);
	EXPECT_EQ(Value1, Value2);
}

TEST(TestVersion, Inequality)
{
	Version Value1(2, 4, 8);
	Version Value2(1, 2, 3);
	EXPECT_NE(Value1, Value2);
}

TEST(TestVersion, Comparison)
{
	Version Value1(1, 0, 0);
	Version Value2(1, 0, 1);
	Version Value3(1, 1, 0);
	Version Value4(2, 0, 0);

	EXPECT_GT(Value2, Value1);
	EXPECT_LT(Value1, Value2);

	EXPECT_GT(Value3, Value1);
	EXPECT_LT(Value1, Value3);

	EXPECT_GT(Value3, Value2);
	EXPECT_LT(Value2, Value3);

	EXPECT_GT(Value4, Value1);
	EXPECT_LT(Value1, Value4);

	EXPECT_GT(Value4, Value2);
	EXPECT_LT(Value2, Value4);

	EXPECT_GT(Value4, Value3);
	EXPECT_LT(Value3, Value4);
}

TEST(TestVersion, ToCompact)
{
	Version Value(17, 2, 42);

	uint32 TrueValue = (17 << 24) + (2 << 12) + (42 << 0);
	EXPECT_EQ(TrueValue, Value.ToCompact<uint32>(12));
}

TEST(TestVersion, ToCompactWithDifferentOffsets)
{
	Version Value(32, 17, 5);

	uint64 TrueValue = (32 << 24) + (17 << 16) + (5 << 4);
	EXPECT_EQ(Value.ToCompact<uint64>(24, 16, 4), TrueValue);
}

TEST(TestVersion, FromCompact)
{
	uint32 Value = (17 << 20) + (12 << 10) + (4 << 0);

	EXPECT_EQ(Version::FromCompact(Value, 10), Version(17, 12, 4));
}

TEST(TestVersion, FromCompactWithDifferentOffsets)
{
	uint64 Value = (30 << 24) + (20 << 16) + (2 << 0);

	EXPECT_EQ(Version::FromCompact(Value, 24, 16, 0), Version(30, 20, 2));
}

TEST(TestVersion, FromString)
{
	auto Value = Version::FromString("1.7.2");
	ASSERT_TRUE(Value.has_value());
	EXPECT_EQ(Value.value(), Version(1, 7, 2));
}

TEST(TestVersion, FromStringWithoutPatch)
{
	auto Value = Version::FromString("1.7");
	ASSERT_TRUE(Value.has_value());
	EXPECT_EQ(Value.value(), Version(1, 7, 0));
}

TEST(TestVersion, FromStringWithoutMinorAndPatch)
{
	auto Value = Version::FromString("3");
	ASSERT_TRUE(Value.has_value());
	EXPECT_EQ(Value.value(), Version(3, 0, 0));
}

TEST(TestVersion, FromStringFail)
{
	EXPECT_FALSE(Version::FromString("").has_value());
	EXPECT_FALSE(Version::FromString("abc").has_value());
	EXPECT_FALSE(Version::FromString(".").has_value());
	EXPECT_FALSE(Version::FromString("..").has_value());
	EXPECT_FALSE(Version::FromString("1.").has_value());
	EXPECT_FALSE(Version::FromString("1.2.").has_value());
	EXPECT_FALSE(Version::FromString("1.2.3.").has_value());
}

TEST(TestVersion, ToString)
{
	auto Value = Version(1, 7, 2);
	EXPECT_EQ(Value.ToString(), "1.7.2");
}

TEST(TestVersion, ToStringWithoutPatch)
{
	auto Value = Version(1, 7, 2);
	EXPECT_EQ(Value.ToStringWithoutPatch(), "1.7");
}
