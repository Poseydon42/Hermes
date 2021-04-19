#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"

namespace Hermes
{
	namespace RenderInterface
	{
		/**
		 * This is a main access point to all rendering API functionality
		 * It is responsible for enumerating available devices and creating them
		 */
		class HERMES_API Instance
		{
			MAKE_NON_COPYABLE(Instance)
		
		public:
			Instance() = default;
			
			virtual ~Instance() = default;
			Instance(Instance&& Other) = default;
			Instance& operator=(Instance&& Other) = default;

			/**
			 * Retrieves a list of all available GPUs in systems, potentially including virtual and CPU-emulated
			 */
			virtual std::vector<DeviceProperties> EnumerateAvailableDevices() = 0;

			/**
			 * Retrieves a handle to a physical device selected from those that were returned by EnumerateAvailableDevices()
			 * @param Index Index of device that you need. This should be retrieved from DeviceProperties struct of required device
			 */
			virtual std::shared_ptr<PhysicalDevice> GetPhysicalDevice(DeviceIndex Index) = 0;
		};
	}
}

