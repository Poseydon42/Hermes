#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"
#include "Math/Vector2.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;

		class HERMES_API VulkanSwapchain : public RenderInterface::Swapchain
		{
			MAKE_NON_COPYABLE(VulkanSwapchain)
		public:
			VulkanSwapchain(VkPhysicalDevice PhysicalDevice, VkDevice InDevice, VkSurfaceKHR Surface, Vec2i Size, uint32 Frames);
			
			~VulkanSwapchain() override;
			VulkanSwapchain(VulkanSwapchain&& Other);
			VulkanSwapchain& operator=(VulkanSwapchain&& Other);

			RenderInterface::ImageFormat GetImageFormat() const override { return (RenderInterface::ImageFormat)SwapchainFormat; }
			Vec2ui GetSize() const override { return Size; }
		private:
			VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
			VkFormat SwapchainFormat = VK_FORMAT_UNDEFINED;
			VkDevice Device;
			Vec2ui Size;
		};
	}
}
