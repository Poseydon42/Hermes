#include <gtest/gtest.h>

#include "Core/UTF8/UTF8Utils.h"

using namespace Hermes;
using namespace UTF8;

TEST(TestUTF8Utils, EncodeOneByte)
{
	auto String = Encode('a');
	EXPECT_EQ(String, "a");
}

TEST(TestUTF8Utils, EncodeTwoBytes)
{
	auto String = Encode(0x0410);
	EXPECT_EQ(String, "\xD0\x90");
}

TEST(TestUTF8Utils, EncodeThreeBytes)
{
	auto String = Encode(0x0F14);
	EXPECT_EQ(String, "\xE0\xBC\x94");
}

TEST(TestUTF8Utils, EncodeFourBytes)
{
	auto String = Encode(0x1F785);
	EXPECT_EQ(String, "\xF0\x9F\x9E\x85");
}

TEST(TestUTF8Utils, Append)
{
	String String;

	Append(String, 'a');
	EXPECT_EQ(String, "a");

	Append(String, 0x0410);
	EXPECT_EQ(String, "a\xD0\x90");

	Append(String, 0x0F14);
	EXPECT_EQ(String, "a\xD0\x90\xE0\xBC\x94");

	Append(String, 0x1F785);
	EXPECT_EQ(String, "a\xD0\x90\xE0\xBC\x94\xF0\x9F\x9E\x85");
}

TEST(TestUTF8Utils, Insert)
{
	String String = " ";

	Insert(String, Begin(String), 0x0410);
	EXPECT_EQ(String, "\xD0\x90 ");

	Insert(String, End(String), 0x0410);
	EXPECT_EQ(String, "\xD0\x90 \xD0\x90");
}

TEST(TestUTF8Utils, Erase)
{
	String String = "a\xD0\x90\xE0\xBC\x94\xF0\x9F\x9E\x85";

	Erase(String, Begin(String) + 1);
	EXPECT_EQ(String, "a\xE0\xBC\x94\xF0\x9F\x9E\x85");

	Erase(String, Begin(String) + 1);
	EXPECT_EQ(String, "a\xF0\x9F\x9E\x85");

	Erase(String, Begin(String) + 1);
	EXPECT_EQ(String, "a");
}

TEST(TestUTF8Utils, EraseRange)
{
	String String = "a\xD0\x90\xE0\xBC\x94\xF0\x9F\x9E\x85";

	Erase(String, Begin(String) + 1, Begin(String) + 4);
	EXPECT_EQ(String, "a");
}
