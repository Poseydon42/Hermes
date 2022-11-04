#pragma once

#include <memory>
#include <optional>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Math/Vector2.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Vulkan/Device.h"
#include "Vulkan/Forward.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	/*
	 * A wrapper around VkSwapchainKHR
	 */
	class HERMES_API Swapchain
	{
		MAKE_NON_COPYABLE(Swapchain)
		MAKE_NON_MOVABLE(Swapchain)

	public:
		Swapchain(std::shared_ptr<Device::VkDeviceHolder> InDevice, const IPlatformWindow& InWindow,
		          uint32 NumFrames);

		~Swapchain();

		/*
		 * Tries to acquire the index of the next swapchain image that the application can draw to. Blocks current thread
		 *
		 * @param Timeout Number of nanoseconds to block the current thread for while waiting for the image to become available
		 * @param Fence Fence object that will be signaled when the presentation engine no
		 *              longer needs the image, so the application can start using it
		 * @param SwapchainWasRecreated Is set to true if the swapchain images were changed (e.g. swapchain was resized)
		 */
		std::optional<uint32> AcquireImage(uint64 Timeout, const Fence& Fence, bool& SwapchainWasRecreated);

		/*
		 * Presents an image with the given index (that must have been previously acquired via AcquireImage()). Blocking
		 *
		 * @param ImageIndex Index of the swapchain image to present
		 * @param SwapchainWasRecreated Is set to true if the swapchain images were changed (e.g. swapchain was resized)
		 */
		void Present(uint32 ImageIndex, bool& SwapchainWasRecreated);

		VkFormat GetImageFormat() const;

		Vec2ui GetDimensions() const;

		/*
		 * Returns a reference to the swapchain image with given index that was previously acquired via AcquireImage()
		 */
		const Image& GetImage(uint32 Index) const;

		uint32 GetImageCount() const;

	private:
		std::shared_ptr<Device::VkDeviceHolder> Device;

		VkSwapchainKHR Handle = VK_NULL_HANDLE;
		VkFormat SwapchainFormat = VK_FORMAT_UNDEFINED;
		std::vector<std::unique_ptr<Image>> Images;
		const IPlatformWindow& Window;
		Vec2ui Dimensions;

		void RecreateSwapchain(uint32 NumFrames);
	};
}
