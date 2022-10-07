#pragma once

#include "Core/Core.h"

namespace Hermes
{
	enum class MaterialPropertyType
	{
		Undefined,
		Float,
	};

	struct HERMES_API MaterialProperty
	{
		/*
		 * Base type of a compound type (e.g. double for dvec3 or float for mat4)
		 */
		MaterialPropertyType Type = MaterialPropertyType::Undefined;

		/*
		 * Number of elements in a compound type in a single dimension (e.g. 2 for vec2 or 4 for mat4)
		 */
		uint32 Width = 1;

		/*
		 * Full size of this property in bytes, without padding
		 */
		size_t Size = 0;

		/*
		 * Offset of the memory address that holds this property in the uniform
		 * buffer relative to the beginning of this uniform buffer
		 */
		size_t Offset = 0;
	};
}
