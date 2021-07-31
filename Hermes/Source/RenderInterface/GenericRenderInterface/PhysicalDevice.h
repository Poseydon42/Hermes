#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/EnumClassOperators.h"

namespace Hermes
{
	namespace RenderInterface
	{
		using DeviceIndex = size_t;
		
		struct DeviceProperties
		{
			ANSIString Name;
			DeviceIndex InternalIndex = 0;
		};

		class Device;

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

			virtual const DeviceProperties& GetProperties() const = 0;

			/**
			 * Creates a logical device from given physical device
			 */
			virtual std::shared_ptr<Device> CreateDevice() = 0;
		};
	}
}
