#include "MaterialProperty.h"

#include "Math/Math.h"

namespace Hermes
{
	size_t GetMaterialPropertySize(MaterialPropertyType Type)
	{
		switch (Type)
		{
		case MaterialPropertyType::Vec4:
			return sizeof(Vec4);
		case MaterialPropertyType::Undefined:
		default:
			HERMES_ASSERT(false);
			return 0;
		}
	}
}
