#pragma once

#include "Core/Core.h"

#define MAKE_NON_COPYABLE(Name) \
	public: \
		Name(const Name&) = delete; \
		Name& operator=(const Name&) = delete; \

#define MAKE_NON_MOVABLE(Name) \
	public: \
		Name(Name&& Other) = delete; \
		Name& operator=(Name&&) = delete; \