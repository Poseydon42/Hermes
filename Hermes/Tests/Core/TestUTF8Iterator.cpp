#include <gtest/gtest.h>

#include "Core/UTF8/UTF8Iterator.h"

using namespace Hermes;
using namespace Hermes::UTF8;

StringView TestString = "\x41\xD0\x91\xE0\xBC\x94\xF0\x9F\x9E\x85";

TEST(TestUTF8Iterator, Comparison)
{
	auto First = Begin(TestString);

	EXPECT_TRUE(First == Begin(TestString));
	EXPECT_FALSE(First != Begin(TestString));
	EXPECT_FALSE(First < Begin(TestString));
	EXPECT_TRUE(First <= Begin(TestString));
	EXPECT_TRUE(First >= Begin(TestString));
	EXPECT_FALSE(First > Begin(TestString));

	auto Second = First + 1;
	EXPECT_FALSE(First == Second);
	EXPECT_TRUE(First != Second);
	EXPECT_TRUE(First < Second);
	EXPECT_TRUE(First <= Second);
	EXPECT_FALSE(First >= Second);
	EXPECT_FALSE(First > Second);

	EXPECT_FALSE(Second < First);
	EXPECT_FALSE(Second <= First);
	EXPECT_TRUE(Second >= First);
	EXPECT_TRUE(Second > First);
}

TEST(TestUTF8Iterator, TestForwardIteration)
{
	auto Iterator = Begin(TestString);
	EXPECT_EQ(*Iterator, 0x0041);
	EXPECT_EQ(Iterator.ToStringViewIterator(TestString), TestString.begin());
	EXPECT_EQ(Iterator, Begin(TestString) + 0);
	++Iterator;

	EXPECT_EQ(*Iterator, 0x0411);
	EXPECT_EQ(Iterator.ToStringViewIterator(TestString), TestString.begin() + 1);
	EXPECT_EQ(Iterator, Begin(TestString) + 1);
	++Iterator;

	EXPECT_EQ(*Iterator, 0x0F14);
	EXPECT_EQ(Iterator.ToStringViewIterator(TestString), TestString.begin() + 3);
	EXPECT_EQ(Iterator, Begin(TestString) + 2);
	++Iterator;

	EXPECT_EQ(*Iterator, 0x1F785);
	EXPECT_EQ(Iterator.ToStringViewIterator(TestString), TestString.begin() + 6);
	EXPECT_EQ(Iterator, Begin(TestString) + 3);
	++Iterator;

	EXPECT_EQ(Iterator.ToStringViewIterator(TestString), TestString.end());
	EXPECT_EQ(Iterator, Begin(TestString) + 4);
}

TEST(TestUTF8Iterator, TestBackwardIteration)
{
	auto Iterator = End(TestString);
	EXPECT_EQ(Iterator.ToStringViewIterator(TestString), TestString.end());
	EXPECT_EQ(Iterator, End(TestString) - 0);

	--Iterator;
	EXPECT_EQ(*Iterator, 0x1F785);
	EXPECT_EQ(Iterator.ToStringViewIterator(TestString), TestString.begin() + 6);
	EXPECT_EQ(Iterator, End(TestString) - 1);

	--Iterator;
	EXPECT_EQ(*Iterator, 0x0F14);
	EXPECT_EQ(Iterator.ToStringViewIterator(TestString), TestString.begin() + 3);
	EXPECT_EQ(Iterator, End(TestString) - 2);

	--Iterator;
	EXPECT_EQ(*Iterator, 0x0411);
	EXPECT_EQ(Iterator.ToStringViewIterator(TestString), TestString.begin() + 1);
	EXPECT_EQ(Iterator, End(TestString) - 3);

	--Iterator;
	EXPECT_EQ(*Iterator, 0x0041);
	EXPECT_EQ(Iterator.ToStringViewIterator(TestString), TestString.begin());
	EXPECT_EQ(Iterator, End(TestString) - 4);
}
