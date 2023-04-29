#pragma once

#include "Core/Core.h"

namespace Hermes
{
	class HERMES_API UTF8StringIterator
	{
	public:
		UTF8StringIterator(const char* InPtr, const char* InEnd);

		UTF8StringIterator& operator++();

		bool operator!=(const UTF8StringIterator& Other) const;

		uint32 operator*() const;

	private:
		const char* Ptr = nullptr;
		const char* End = nullptr;

		uint32 ReadByte(size_t Offset) const;
	};

	class HERMES_API UTF8StringView
	{
	public:
		explicit UTF8StringView(StringView String);

		UTF8StringIterator begin() const;
		UTF8StringIterator end() const;

		UTF8StringIterator cbegin() const;
		UTF8StringIterator cend() const;

	private:
		const char* Begin = nullptr;
		const char* End = nullptr;
	};
}
