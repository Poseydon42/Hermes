#pragma once

#include <vector>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Vulkan/Forward.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	/*
	 * Struct containing properties of a GPU installed in the system
	 */
	struct DeviceProperties
	{
		String Name;
			
		bool AnisotropySupport = false;
		float MaxAnisotropyLevel = 0.0f;

		VkPhysicalDeviceType Type = VK_PHYSICAL_DEVICE_TYPE_OTHER;
	};

	/*
	 * Represents an entry point to a Vulkan API.
	 *
	 * Instance is the first object that needs to be created during rendering initialization and
	 * it lets you enumerate available devices and create an interface to a logical device
	 */
	class HERMES_API Instance
	{
		MAKE_NON_COPYABLE(Instance)
		MAKE_NON_MOVABLE(Instance)
		ADD_DEFAULT_DESTRUCTOR(Instance)

	public:
		Instance(const IPlatformWindow& InWindow);

		/*
		 * Returns a list of properties of all available devices installed in the system
		 */
		const std::vector<DeviceProperties>& EnumerateAvailableDevices() const;

		/*
		 * Creates a Device instance from a device with a given index
		 *
		 * @param DeviceIndex Index of the device in a vector returned by EnumerateAvailableDevices()
		 */
		std::unique_ptr<Device> CreateDevice(size_t DeviceIndex) const;

		VkInstance GetInstance() const { return Holder->Instance; }
			
	private:
		struct VkInstanceHolder
		{
			VkInstance Instance = VK_NULL_HANDLE;
			VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
			VkSurfaceKHR Surface = VK_NULL_HANDLE;

			ADD_DEFAULT_CONSTRUCTOR(VkInstanceHolder)
			MAKE_NON_COPYABLE(VkInstanceHolder)
			MAKE_NON_MOVABLE(VkInstanceHolder)

			~VkInstanceHolder();
		};

		std::shared_ptr<VkInstanceHolder> Holder;
		const IPlatformWindow& Window;

		std::vector<VkPhysicalDevice> AvailableDevices;
		std::vector<DeviceProperties> AvailableDeviceProperties;

		void CreateDebugMessenger();

		friend class Device;
	};
}
