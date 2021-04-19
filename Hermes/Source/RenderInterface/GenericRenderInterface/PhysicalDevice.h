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

		using DeviceIndex = size_t;
		
		struct DeviceProperties
		{
			ANSIString Name;
			std::vector<QueueFamilyProperties> QueueFamilies;
			DeviceIndex InternalIndex = 0;
		};

		/**
		 * A wrapper around handle to GPU
		 * Does not do anything but create an actual 'logical' device that is a single access point to all GPU features
		 */
		class HERMES_API PhysicalDevice
		{
			MAKE_NON_COPYABLE(PhysicalDevice)
		public:
			PhysicalDevice() = default;
			virtual ~PhysicalDevice() = default;
			PhysicalDevice(PhysicalDevice&&) = default;
			PhysicalDevice& operator=(PhysicalDevice&&) = default;
		};
	}
}
