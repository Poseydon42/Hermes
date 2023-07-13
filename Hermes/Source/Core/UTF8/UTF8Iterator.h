#pragma once

#include "Core/Core.h"

namespace Hermes::UTF8
{
	class HERMES_API Iterator
	{
	public:
		Iterator(String::const_iterator InPtr, String::const_iterator InStart, String::const_iterator InEnd);

		Iterator(StringView::const_iterator InPtr, StringView::const_iterator InStart, StringView::const_iterator InEnd);

		Iterator& operator++();
		Iterator& operator--();

		Iterator operator+(size_t Difference) const;
		Iterator operator-(size_t Difference) const;

		bool operator==(const Iterator& Other) const;
		bool operator!=(const Iterator& Other) const;

		uint32 operator*() const;

		String::const_iterator ToStringIterator(const String& String) const;
		StringView::const_iterator ToStringViewIterator(StringView View) const;

	private:
		const char* Ptr;
		const char* Start;
		const char* End;

		uint32 ReadByte(size_t Offset) const;
	};

	Iterator Begin(const String& String);
	Iterator Begin(StringView String);

	Iterator End(const String& String);
	Iterator End(StringView String);
}
