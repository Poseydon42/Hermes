#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"

namespace Hermes
{
	namespace RenderInterface
	{
		using DeviceIndex = size_t;
		
		struct DeviceProperties
		{
			ANSIString Name;
			DeviceIndex InternalIndex = 0;
			bool AnisotropySupport = false;
			float MaxAnisotropyLevel = 0;
		};

		class Device;

		/**
		 * A wrapper around handle to GPU
		 * Does not do anything but create an actual 'logical' device that is a single access point to all GPU features
		 */
		class HERMES_API PhysicalDevice
		{
			MAKE_NON_COPYABLE(PhysicalDevice)
			ADD_DEFAULT_CONSTRUCTOR(PhysicalDevice)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(PhysicalDevice)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(PhysicalDevice)
		public:

			virtual const DeviceProperties& GetProperties() const = 0;

			/**
			 * Creates a logical device from given physical device
			 */
			virtual std::shared_ptr<Device> CreateDevice() const = 0;
		};
	}
}
