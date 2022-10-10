#pragma once

#include "Core/Core.h"

namespace Hermes
{
	/*
	 * Data types that can be reflected from a shader
	 */
	enum class MaterialPropertyDataType
	{
		Undefined,
		Float,
	};

	/*
	 * Types of individual properties that can be reflected from a shader
	 */
	enum class MaterialPropertyType
	{
		Undefined,
		Value,
		Vector,
		Matrix,
		Texture
	};

	struct HERMES_API MaterialProperty
	{
		/*
		 * Type of the property
		 */
		MaterialPropertyType Type = MaterialPropertyType::Undefined;

		/*
		 * Data type of a compound type (e.g. double for dvec3 or float for mat4)
		 *
		 * Undefined for non-value objects (e.g. textures, samplers etc.)
		 */
		MaterialPropertyDataType DataType = MaterialPropertyDataType::Undefined;

		/*
		 * Number of elements in a compound type in a single dimension (e.g. 2 for vec2 or 4 for mat4)
		 *
		 * NOTE: only square matrices are supported, therefore we only need one variable to describe
		 *       the size of any compound type.
		 */
		uint32 Width = 1;

		/*
		 * Full size of this property in bytes, without padding
		 *
		 * Must always be zero for non-value properties (e.g. textures, samplers etc.)
		 */
		size_t Size = 0;

		/*
		 * Offset of the memory address that holds this property in the uniform
		 * buffer relative to the beginning of this uniform buffer
		 */
		size_t Offset = 0;
	};
}
