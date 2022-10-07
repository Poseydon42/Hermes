#pragma once

#include "Core/Core.h"

namespace Hermes
{
	enum class MaterialPropertyType
	{
		Undefined = 0x00,
		Vec4 = 0xF1,
	};

	size_t HERMES_API GetMaterialPropertySize(MaterialPropertyType Type);

	struct MaterialProperty
	{
		String Name;
		MaterialPropertyType Type;
		size_t Offset;
	};
}
