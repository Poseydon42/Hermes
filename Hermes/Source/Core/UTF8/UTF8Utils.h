#pragma once

#include "UTF8Iterator.h"
#include "Core/Core.h"

/**
 * Some functions to manipulate strings encoded with UTF8.
 * These can be used for ASCII strings too, but the default String functions will be much faster.
 *
 * NOTE: the behaviour of these functions is undefined if the input is an invalid UTF8 string or
 * an invalid code point. In most cases, the program will crash on assert, read past the end of a
 * buffer or will return garbage.
 */
namespace Hermes::UTF8
{
	void Append(String& String, uint32 CodePoint);

	void Insert(String& String, const Iterator& Position, uint32 CodePoint);

	void Erase(String& String, const Iterator& Position);

	void Erase(String& String, const Iterator& Begin, const Iterator& End);

	size_t Length(const String& String);
	size_t Length(StringView String);
	size_t Length(Iterator Begin, Iterator End);

	String Encode(uint32 CodePoint);
}