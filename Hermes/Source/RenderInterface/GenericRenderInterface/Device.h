#pragma once

#include "Core/Core.h"
#include "Core/Misc/EnumClassOperators.h"

namespace Hermes
{
	namespace RenderInterface
	{
		enum class QueueFamilyType
		{
			Graphics = 0x01,
			Transfer = 0x02
		};
		ENUM_CLASS_OPERATORS(QueueFamilyType);

		struct QueueFamilyProperties
		{
			QueueFamilyType Type;
			uint32_t Count;
		};
		
		struct DeviceProperties
		{
			ANSIString Name;
			std::vector<QueueFamilyProperties> QueueFamilies;
		};
	}
}
