#include "VulkanSwapchain.h"

#include <functional>
#include <memory>
#include <utility>

#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanImage.h"
#include "Math/Math.h"

namespace Hermes
{
	namespace Vulkan
	{
		static VkSurfaceFormatKHR SelectSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& Formats)
		{
			for (const auto& Format : Formats)
			{
				if (Format.format == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					return Format;
				}
			}
			return Formats[0];
		}

		static VkPresentModeKHR SelectPresentMode(const std::vector<VkPresentModeKHR>& /*Modes*/)
		{
			// TODO : actually select most appropriate mode
			return VK_PRESENT_MODE_FIFO_KHR; // Vulkan guarantees that at least this one mode should be present
		}

		static VkExtent2D SelectExtent(const VkSurfaceCapabilitiesKHR& Capabilities, uint32 Width, uint32 Height)
		{
			if (Capabilities.currentExtent.width != UINT32_MAX)
			{
				return Capabilities.currentExtent;
			}
			else
			{
				VkExtent2D Result;
				Result.width  = Math::Clamp(Capabilities.minImageExtent.width , Capabilities.maxImageExtent.width , Width);
				Result.height = Math::Clamp(Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height, Height);
				return Result;
			}
		}
		
		VulkanSwapchain::VulkanSwapchain(std::shared_ptr<VulkanDevice> InDevice, VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface, Vec2i InSize, uint32 Frames)
			: Device(std::move(InDevice))
		{
			HERMES_ASSERT(InSize.X > 0);
			HERMES_ASSERT(InSize.Y > 0);
			HERMES_ASSERT(Frames > 0);
			
			VkSurfaceCapabilitiesKHR Capabilities = {};
			VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &Capabilities));

			std::vector<VkSurfaceFormatKHR> Formats;
			uint32 FormatCount = 0;
			VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, 0));
			HERMES_ASSERT_LOG(FormatCount > 0, L"vkGetPhysicalDeviceSurfaceFormatsKHR returned 0 available formats.");
			Formats.resize(FormatCount);
			VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, Formats.data()));

			std::vector<VkPresentModeKHR> PresentModes;
			uint32 PresentModeCount = 0;
			VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, 0));
			HERMES_ASSERT_LOG(PresentModeCount > 0, L"vkGetPhysicalDeviceSurfacePresentModesKHR returned 0 available present modes.");
			PresentModes.resize(PresentModeCount);
			VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, PresentModes.data()));
			
			
			// At this point we assume that device supports swapchain extension
			// and that it's enabled
			VkSwapchainCreateInfoKHR CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			CreateInfo.surface = Surface;
			CreateInfo.presentMode = SelectPresentMode(PresentModes);
			CreateInfo.imageExtent = SelectExtent(Capabilities, InSize.X, InSize.Y);
			if (Capabilities.maxImageCount == 0)
				CreateInfo.minImageCount = Math::Min(Capabilities.minImageCount, Frames);
			else
				CreateInfo.minImageCount = Math::Clamp(Capabilities.minImageCount, Capabilities.maxImageCount, Frames);
			auto SurfaceFormat = SelectSurfaceFormat(Formats);
			CreateInfo.imageFormat = SurfaceFormat.format;
			CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
			CreateInfo.imageArrayLayers = 1;
			CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			CreateInfo.preTransform = Capabilities.currentTransform;
			CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			CreateInfo.clipped = VK_TRUE;
			CreateInfo.oldSwapchain = VK_NULL_HANDLE;

			VK_CHECK_RESULT(vkCreateSwapchainKHR(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &Swapchain));
			SwapchainFormat = CreateInfo.imageFormat;
			Size = { CreateInfo.imageExtent.width, CreateInfo.imageExtent.height };
			uint32 ImageCount;
			vkGetSwapchainImagesKHR(Device->GetDevice(), Swapchain, &ImageCount, nullptr);
			std::vector<VkImage> ImageHandles(ImageCount, VK_NULL_HANDLE);
			vkGetSwapchainImagesKHR(Device->GetDevice(), Swapchain, &ImageCount, ImageHandles.data());
			Images.reserve(ImageCount);
			for (uint32 Index = 0; Index < ImageCount; Index++)
			{
				Images.push_back(std::make_shared<VulkanImage>(Device, ImageHandles[Index], SwapchainFormat, Size));
			}
		}

		VulkanSwapchain::~VulkanSwapchain()
		{
			vkDestroySwapchainKHR(Device->GetDevice(), Swapchain, GVulkanAllocator);
		}

		VulkanSwapchain::VulkanSwapchain(VulkanSwapchain&& Other)
		{
			*this = std::move(Other);
		}

		VulkanSwapchain& VulkanSwapchain::operator=(VulkanSwapchain&& Other)
		{
			std::swap(Other.Swapchain, Swapchain);
			std::swap(Other.Device, Device);
			std::swap(Other.Size, Size);
			std::swap(Other.SwapchainFormat, SwapchainFormat);

			return *this;
		}
	}
}
