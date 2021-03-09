#pragma once

#include "Core/Core.h"

namespace Hermes
{
	class INonCopyable
	{
	public:
		INonCopyable() = default;
		virtual ~INonCopyable() = default;

		INonCopyable(const INonCopyable&) = delete;
		INonCopyable& operator=(const INonCopyable&) = delete;

		INonCopyable(INonCopyable&&) = default;
		INonCopyable& operator=(INonCopyable&&) = default;
	};
}
