#include "UTF8StringView.h"

namespace Hermes
{
	UTF8StringIterator::UTF8StringIterator(const char* InPtr, const char* InEnd)
		: Ptr(InPtr)
		, End(InEnd)
	{
	}

	UTF8StringIterator& UTF8StringIterator::operator++()
	{
		auto NextCodepoint = this->operator*();
		if (NextCodepoint <= 0x007F)
			Ptr += 1;
		else if (NextCodepoint <= 0x07FF)
			Ptr += 2;
		else if (NextCodepoint <= 0xFFFF)
			Ptr += 3;
		else
			Ptr += 4;

		return *this;
	}

	bool UTF8StringIterator::operator!=(const UTF8StringIterator& Other) const
	{
		HERMES_ASSERT(End == Other.End);
		return Ptr != Other.Ptr;
	}

	uint32 UTF8StringIterator::operator*() const
	{
		auto FirstByte = ReadByte(0);
		if ((FirstByte & 0b10000000) == 0)
			return FirstByte;

		auto SecondByte = ReadByte(1);
		HERMES_ASSERT((SecondByte & 0b11000000) == 0b10000000);
		if ((FirstByte & 0b11100000) == 0b11000000)
		{
			return ((FirstByte & 0b11111) << 6) | ((SecondByte & 0b111111) << 0);
		}
		
		auto ThirdByte = ReadByte(2);
		HERMES_ASSERT((ThirdByte & 0b11000000) == 0b10000000);
		if ((FirstByte & 0b11110000) == 0b11100000)
		{
			return ((FirstByte & 0b1111) << 12) | ((SecondByte & 0b111111) << 6) | ((ThirdByte & 0b111111) << 0);
		}

		auto FourthByte = ReadByte(3);
		HERMES_ASSERT((FourthByte & 0b11000000) == 0b10000000);
		if ((FirstByte & 0b11111000) == 0b11110000)
		{
			return ((FirstByte & 0b1111) << 18) | ((SecondByte & 0b111111) << 12) | ((ThirdByte & 0b111111) << 6) | ((FourthByte & 0b111111) << 0);
		}

		HERMES_ASSERT(false);
	}

	uint32 UTF8StringIterator::ReadByte(size_t Offset) const
	{
		HERMES_ASSERT(Ptr + Offset < End);
		return Ptr[Offset];
	}

	UTF8StringView::UTF8StringView(StringView String)
		: Begin(String.data())
		, End(String.data() + String.size())
	{
	}

	UTF8StringIterator UTF8StringView::begin() const
	{
		return { Begin, End };
	}

	UTF8StringIterator UTF8StringView::end() const
	{
		return { End, End };
	}

	UTF8StringIterator UTF8StringView::cbegin() const
	{
		return { Begin, End };
	}

	UTF8StringIterator UTF8StringView::cend() const
	{
		return { End, End };
	}
}
