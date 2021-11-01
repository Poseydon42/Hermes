#pragma once

#include <memory>
#include <optional>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"
#include "Math/Vector2.h"
#include "RenderInterface/Vulkan/VulkanCommonTypes.h"
#include "RenderInterface/Vulkan/VulkanImage.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
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
			VulkanSwapchain(std::shared_ptr<const VulkanDevice> InDevice, VkPhysicalDevice InPhysicalDevice, VkSurfaceKHR InSurface, std::weak_ptr<const IPlatformWindow> InWindow, uint32 NumFrames);
			
			~VulkanSwapchain() override;
			VulkanSwapchain(VulkanSwapchain&& Other);
			VulkanSwapchain& operator=(VulkanSwapchain&& Other);

			RenderInterface::DataFormat GetImageFormat() const override { return VkFormatToDataFormat(SwapchainFormat); }
			
			Vec2ui GetSize() const override { return Size; }
			
			std::shared_ptr<RenderInterface::Image> GetImage(uint32 Index) const override { return Images[Index]; }

			std::optional<uint32> AcquireImage(uint64 Timeout, const RenderInterface::Fence& Fence, bool& SwapchainWasRecreated) override;
			
			void Present(uint32 ImageIndex, bool& SwapchainWasRecreated) override;

			uint32 GetImageCount() const override { return (uint32)Images.size(); }
			
		private:
			void RecreateSwapchain(uint32 NumFrames);

			VkPhysicalDevice PhysicalDevice;
			VkSurfaceKHR Surface;
			VkSwapchainKHR Swapchain;
			VkFormat SwapchainFormat;
			std::shared_ptr<const VulkanDevice> Device;
			std::weak_ptr<const IPlatformWindow> Window;
			std::vector<std::shared_ptr<VulkanImage>> Images;
			Vec2ui Size;
		};
	}
}
