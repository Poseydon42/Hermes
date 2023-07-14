#include "UTF8Utils.h"

namespace Hermes::UTF8
{
	void Append(String& String, uint32 CodePoint)
	{
		String += Encode(CodePoint);
	}

	void Insert(String& String, const Iterator& Position, uint32 CodePoint)
	{
		auto StringIterator = Position.ToStringIterator(String);
		auto EncodedCodePoint = Encode(CodePoint);
		String.insert(StringIterator, EncodedCodePoint.begin(), EncodedCodePoint.end());
	}

	void Erase(String& String, const Iterator& Position)
	{
		auto StringBeginIterator = Position.ToStringIterator(String);
		auto StringEndIterator = (Position + 1).ToStringIterator(String);
		String.erase(StringBeginIterator, StringEndIterator);
	}

	void Erase(String& String, const Iterator& Begin, const Iterator& End)
	{
		auto StringBeginIterator = Begin.ToStringIterator(String);
		auto StringEndIterator = End.ToStringIterator(String);
		String.erase(StringBeginIterator, StringEndIterator);
	}

	size_t Length(const String& String)
	{
		return Length(Begin(String), End(String));
	}

	size_t Length(StringView String)
	{
		return Length(Begin(String), End(String));
	}

	size_t Length(Iterator Begin, Iterator End)
	{
		size_t Counter = 0;
		while (Begin < End)
		{
			Counter++;
			++Begin;
		}
		return Counter;
	}

	String Encode(uint32 CodePoint)
	{
		if (CodePoint <= 0x007F)
			return { static_cast<char>(CodePoint) };
		if (CodePoint >= 0x0080 && CodePoint <= 0x07FF)
		{
			auto FirstByte  = static_cast<char>(0xC0 | ((CodePoint >> 6) & 0x1F));
			auto SecondByte = static_cast<char>(0x80 | ((CodePoint >> 0) & 0x3F));
			return { FirstByte, SecondByte };
		}
		if (CodePoint >= 0x0800 && CodePoint <= 0xFFFF)
		{
			auto FirstByte  = static_cast<char>(0xE0 | ((CodePoint >> 12) & 0x0F));
			auto SecondByte = static_cast<char>(0x80 | ((CodePoint >> 6 ) & 0x3F));
			auto ThirdByte  = static_cast<char>(0x80 | ((CodePoint >> 0 ) & 0x3F));
			return { FirstByte, SecondByte, ThirdByte };
		}
		if (CodePoint >= 0x10000 && CodePoint <= 0x10FFFF)
		{
			auto FirstByte  = static_cast<char>(0xF0 | ((CodePoint >> 18) & 0x0F));
			auto SecondByte = static_cast<char>(0x80 | ((CodePoint >> 12) & 0x3F));
			auto ThirdByte  = static_cast<char>(0x80 | ((CodePoint >> 6 ) & 0x3F));
			auto FourthByte = static_cast<char>(0x80 | ((CodePoint >> 0 ) & 0x3F));
			return { FirstByte, SecondByte, ThirdByte, FourthByte };
		}
		HERMES_ASSERT(false);
	}
}
