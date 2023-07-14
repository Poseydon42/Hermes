#include "UTF8Iterator.h"

namespace Hermes::UTF8
{
	Iterator::Iterator(String::const_iterator InPtr, String::const_iterator InStart, String::const_iterator InEnd)
		: Ptr(&*InStart + (InPtr - InStart))
		, Start(&*InStart)
		, End(Start + (InEnd - InStart))
	{
	}

	Iterator::Iterator(StringView::const_iterator InPtr, StringView::const_iterator InStart, StringView::const_iterator InEnd)
		: Ptr(&*InStart + (InPtr - InStart))
		, Start(&*InStart)
		, End(Start + (InEnd - InStart))
	{
	}

	Iterator& Iterator::operator++()
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

	Iterator& Iterator::operator--()
	{
		do
		{
			if (Ptr < Start)
				HERMES_ASSERT(false);
			Ptr--;
		} while ((*Ptr & 0xC0) == 0x80);
		return *this;
	}

	Iterator Iterator::operator+(size_t Difference) const
	{
		Iterator Result = *this;
		while (Difference--)
			++Result;
		return Result;
	}

	Iterator Iterator::operator-(size_t Difference) const
	{
		Iterator Result = *this;
		while (Difference--)
			--Result;
		return Result;
	}

	bool Iterator::operator==(const Iterator& Other) const
	{
		HERMES_ASSERT(Start == Other.Start && End == Other.End);
		return Ptr == Other.Ptr;
	}

	bool Iterator::operator!=(const Iterator& Other) const
	{
		return !this->operator==(Other);
	}

	bool Iterator::operator<(const Iterator& Other) const
	{
		HERMES_ASSERT(Start == Other.Start && End == Other.End);
		return Ptr < Other.Ptr;
	}

	bool Iterator::operator<=(const Iterator& Other) const
	{
		return this->operator<(Other) || this->operator==(Other);
	}

	bool Iterator::operator>(const Iterator& Other) const
	{
		return !this->operator<=(Other);
	}

	bool Iterator::operator>=(const Iterator& Other) const
	{
		return !this->operator<(Other);
	}

	uint32 Iterator::operator*() const
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

	String::const_iterator Iterator::ToStringIterator(const String& String) const
	{
		HERMES_ASSERT(&*String.begin() == Start && &*String.begin() + String.length() == End);
		HERMES_ASSERT(Ptr >= Start && Ptr <= End);
		return String.begin() + (Ptr - Start);
	}

	StringView::const_iterator Iterator::ToStringViewIterator(StringView View) const
	{
		HERMES_ASSERT(&*View.begin() == Start && &*View.begin() + View.length() == End);
		HERMES_ASSERT(Ptr >= Start && Ptr <= End);
		return View.begin() + (Ptr - Start);
	}

	uint32 Iterator::ReadByte(size_t Offset) const
	{
		HERMES_ASSERT(Ptr + Offset >= Start && Ptr + Offset < End);
		return Ptr[Offset];
	}

	Iterator Begin(const String& String)
	{
		return { String.begin(), String.begin(), String.end() };
	}

	Iterator Begin(StringView String)
	{
		return { String.begin(), String.begin(), String.end() };
	}

	Iterator End(const String& String)
	{
		return { String.end(), String.begin(), String.end() };
	}

	Iterator End(StringView String)
	{
		return { String.end(), String.begin(), String.end() };
	}
}
