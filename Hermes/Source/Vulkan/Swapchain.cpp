#include "Swapchain.h"

#include <functional>
#include <memory>
#include <utility>

#include "Core/Profiling.h"
#include "Math/Math.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Image.h"

namespace Hermes::Vulkan
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
			Result.width = Math::Clamp(Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width, Width);
			Result.height = Math::Clamp(Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height, Height);
			return Result;
		}
	}

	Swapchain::Swapchain(std::shared_ptr<Device::VkDeviceHolder> InDevice, const IPlatformWindow& InWindow,
	                     uint32 NumFrames)
		: Device(std::move(InDevice))
		, Window(InWindow)
	{
		RecreateSwapchain(NumFrames);
	}

	Swapchain::~Swapchain()
	{
		vkDestroySwapchainKHR(Device->Device, Handle, GVulkanAllocator);
	}

	VkFormat Swapchain::GetImageFormat() const
	{
		return SwapchainFormat;
	}

	Vec2ui Swapchain::GetDimensions() const
	{
		return Dimensions;
	}

	const Image& Swapchain::GetImage(uint32 Index) const
	{
		return *Images[Index];
	}

	std::optional<uint32> Swapchain::AcquireImage(uint64 Timeout, const Fence& Fence, bool& SwapchainWasRecreated)
	{
		HERMES_PROFILE_FUNC();
		uint32 Result;
		VkResult Error = vkAcquireNextImageKHR(Device->Device, Handle, Timeout, VK_NULL_HANDLE, Fence.GetFence(),
		                                       &Result);
		if (Error == VK_SUCCESS)
			return Result;
		if (Error == VK_SUBOPTIMAL_KHR || Error == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapchain(static_cast<uint32>(Images.size()));
			SwapchainWasRecreated = true;
			return {};
		}
		if (Error == VK_TIMEOUT || Error == VK_NOT_READY)
			return {};
		VK_CHECK_RESULT(Error); // This will trigger assert
		return {};
	}

	void Swapchain::Present(uint32 ImageIndex, bool& SwapchainWasRecreated)
	{
		HERMES_PROFILE_FUNC();
		VkPresentInfoKHR Info = {};
		VkResult Result;
		Info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		Info.pResults = &Result;
		Info.pImageIndices = &ImageIndex;
		Info.pSwapchains = &Handle;
		Info.swapchainCount = 1;
		Info.waitSemaphoreCount = 0;
		{
			vkQueuePresentKHR(Device->PresentationQueue, &Info);
			HERMES_PROFILE_FRAME();
		}
		if (Info.pResults[0] == VK_SUBOPTIMAL_KHR || Info.pResults[0] == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapchain(static_cast<uint32>(Images.size()));
			SwapchainWasRecreated = true;
			return;
		}
		VK_CHECK_RESULT(Info.pResults[0]);
	}

	uint32 Swapchain::GetImageCount() const
	{
		return static_cast<uint32>(Images.size());
	}

	void Swapchain::RecreateSwapchain(uint32 NumFrames)
	{
		Images.clear();

		auto NewDimensions = Window.GetSize();
		if (!(NewDimensions.X && NewDimensions.Y))
			return;

		VkSurfaceCapabilitiesKHR Capabilities = {};
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device->PhysicalDevice, Device->Instance->Surface,
			                &Capabilities));

		std::vector<VkSurfaceFormatKHR> Formats;
		uint32 FormatCount = 0;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(Device->PhysicalDevice, Device->Instance->Surface, &
			                FormatCount, nullptr));
		HERMES_ASSERT_LOG(FormatCount > 0, "vkGetPhysicalDeviceSurfaceFormatsKHR returned 0 available formats.");
		Formats.resize(FormatCount);
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(Device->PhysicalDevice, Device->Instance->Surface, &
			                FormatCount, Formats.data()));

		std::vector<VkPresentModeKHR> PresentModes;
		uint32 PresentModeCount = 0;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(Device->PhysicalDevice, Device->Instance->Surface, &
			                PresentModeCount, nullptr));
		HERMES_ASSERT_LOG(PresentModeCount > 0,
		                  "vkGetPhysicalDeviceSurfacePresentModesKHR returned 0 available present modes.");
		PresentModes.resize(PresentModeCount);
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(Device->PhysicalDevice, Device->Instance->Surface, &
			                PresentModeCount, PresentModes.data()));

		// At this point we assume that device supports swapchain extension
		// and that it's enabled
		VkSwapchainCreateInfoKHR CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		CreateInfo.surface = Device->Instance->Surface;
		CreateInfo.presentMode = SelectPresentMode(PresentModes);
		CreateInfo.imageExtent = SelectExtent(Capabilities, NewDimensions.X, NewDimensions.Y);
		if (Capabilities.maxImageCount == 0)
			CreateInfo.minImageCount = Math::Min(Capabilities.minImageCount, NumFrames);
		else
			CreateInfo.minImageCount = Math::Clamp(Capabilities.minImageCount, Capabilities.maxImageCount, NumFrames);
		auto SurfaceFormat = SelectSurfaceFormat(Formats);
		CreateInfo.imageFormat = SurfaceFormat.format;
		CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
		CreateInfo.imageArrayLayers = 1;
		CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		CreateInfo.preTransform = Capabilities.currentTransform;
		CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		CreateInfo.clipped = VK_TRUE;
		CreateInfo.oldSwapchain = Handle;

		VK_CHECK_RESULT(vkCreateSwapchainKHR(Device->Device, &CreateInfo, GVulkanAllocator, &Handle));
		SwapchainFormat = CreateInfo.imageFormat;
		Dimensions = { CreateInfo.imageExtent.width, CreateInfo.imageExtent.height };
		uint32 ImageCount;
		vkGetSwapchainImagesKHR(Device->Device, Handle, &ImageCount, nullptr);
		std::vector<VkImage> ImageHandles(ImageCount, VK_NULL_HANDLE);
		vkGetSwapchainImagesKHR(Device->Device, Handle, &ImageCount, ImageHandles.data());
		Images.reserve(ImageCount);
		for (uint32 Index = 0; Index < ImageCount; Index++)
		{
			Images.push_back(std::make_unique<Image>(Device, ImageHandles[Index], SwapchainFormat, Dimensions, false));
		}
	}
}
