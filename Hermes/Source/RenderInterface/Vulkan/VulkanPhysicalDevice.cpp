#include "VulkanPhysicalDevice.h"

#include "VulkanInstance.h"
#include "RenderInterface/Vulkan/VulkanDevice.h"

namespace Hermes
{
	namespace Vulkan
	{
		RenderInterface::DeviceProperties GetPhysicalDeviceProperties(VkPhysicalDevice Device)
		{
			RenderInterface::DeviceProperties Result;
			
			VkPhysicalDeviceProperties Properties;

			vkGetPhysicalDeviceProperties(Device, &Properties);
			Result.Name = Properties.deviceName;
			Result.MaxAnisotropyLevel = Properties.limits.maxSamplerAnisotropy;

			VkPhysicalDeviceFeatures Features;
			vkGetPhysicalDeviceFeatures(Device, &Features);
			Result.AnisotropySupport = Features.samplerAnisotropy;

			return Result;
		}

		VulkanPhysicalDevice::VulkanPhysicalDevice(VulkanPhysicalDevice&& Other)
		{
			*this = std::move(Other);
		}

		VulkanPhysicalDevice& VulkanPhysicalDevice::operator=(VulkanPhysicalDevice&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Instance, Other.Instance);
			std::swap(Surface, Other.Surface);
			std::swap(Properties, Other.Properties);
			return *this;
		}

		const RenderInterface::DeviceProperties& VulkanPhysicalDevice::GetProperties() const
		{
			return Properties;
		}

		std::shared_ptr<RenderInterface::Device> VulkanPhysicalDevice::CreateDevice() const
		{
			return std::make_shared<VulkanDevice>(Device, Instance, Surface, Window);
		}

		VkInstance VulkanPhysicalDevice::GetInstance() const
		{
			return Instance->GetInstance();
		}

		VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice InDevice, std::shared_ptr<const VulkanInstance> InInstance, VkSurfaceKHR InSurface, std::weak_ptr<const IPlatformWindow> InWindow)
			: Device(InDevice)
			, Instance(std::move(InInstance))
			, Surface(InSurface)
			, Properties(GetPhysicalDeviceProperties(Device))
			, Window(std::move(InWindow))
		{
		}
	}
}
