#include "VulkanRenderTarget.h"

#include "RenderInterface/Vulkan/VulkanImage.h"
#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanRenderPass.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanRenderTarget::VulkanRenderTarget(std::shared_ptr<const VulkanDevice> InDevice, const RenderInterface::RenderPass& InRenderPass, const std::vector<const RenderInterface::Image*>& InAttachments, Vec2ui InSize)
			: Device(std::move(InDevice))
			, Framebuffer(VK_NULL_HANDLE)
			, ImageCount(static_cast<uint32>(InAttachments.size()))
			, Size(InSize)
		{
			std::vector<VkImageView> AttachmentViews(InAttachments.size(), VK_NULL_HANDLE);
			for (size_t Index = 0; Index < AttachmentViews.size(); Index++)
			{
				AttachmentViews[Index] = reinterpret_cast<const VulkanImage*>(InAttachments[Index])->GetDefaultView();
			}

			VkFramebufferCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			CreateInfo.attachmentCount = static_cast<uint32>(AttachmentViews.size());
			CreateInfo.pAttachments = AttachmentViews.data();
			CreateInfo.width = Size.X;
			CreateInfo.height = Size.Y;
			CreateInfo.layers = 1;
			CreateInfo.renderPass = reinterpret_cast<const VulkanRenderPass&>(InRenderPass).GetRenderPass();

			VK_CHECK_RESULT(vkCreateFramebuffer(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &Framebuffer));
		}

		VulkanRenderTarget::~VulkanRenderTarget()
		{
			vkDestroyFramebuffer(Device->GetDevice(), Framebuffer, GVulkanAllocator);
		}

		VulkanRenderTarget::VulkanRenderTarget(VulkanRenderTarget&& Other)
		{
			*this = std::move(Other);
		}

		VulkanRenderTarget& VulkanRenderTarget::operator=(VulkanRenderTarget&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Framebuffer, Other.Framebuffer);
			std::swap(ImageCount, Other.ImageCount);
			std::swap(Size, Other.Size);

			return *this;
		}

		Vec2ui VulkanRenderTarget::GetSize() const
		{
			return Size;
		}

		uint32 VulkanRenderTarget::GetImageCount() const
		{
			return ImageCount;
		}

		VkFramebuffer VulkanRenderTarget::GetFramebuffer() const
		{
			return Framebuffer;
		}
	}
}
