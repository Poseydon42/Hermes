#include "VulkanRenderTarget.h"

#include "RenderInterface/Vulkan/VulkanImage.h"
#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanRenderPass.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanRenderTarget::VulkanRenderTarget(std::shared_ptr<const VulkanDevice> InDevice, const std::shared_ptr<RenderInterface::RenderPass>& InRenderPass, std::vector<std::shared_ptr<RenderInterface::Image>> InAttachments, Vec2ui InSize)
			: Device(std::move(InDevice))
			, RenderPass(std::reinterpret_pointer_cast<VulkanRenderPass>(InRenderPass))
			, Framebuffer(VK_NULL_HANDLE)
			, Size(InSize)
		{
			Attachments.reserve(InAttachments.size());
			for (const auto& Attachment : InAttachments)
			{
				Attachments.push_back(std::reinterpret_pointer_cast<VulkanImage>(Attachment));
			}

			std::vector<VkImageView> AttachmentViews(Attachments.size(), VK_NULL_HANDLE);
			for (size_t Index = 0; Index < AttachmentViews.size(); Index++)
			{
				AttachmentViews[Index] = Attachments[Index]->GetDefaultView();
			}

			VkFramebufferCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			CreateInfo.attachmentCount = (uint32)AttachmentViews.size();
			CreateInfo.pAttachments = AttachmentViews.data();
			CreateInfo.width = Size.X;
			CreateInfo.height = Size.Y;
			CreateInfo.layers = 1;
			CreateInfo.renderPass = RenderPass->GetRenderPass();

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
			std::swap(RenderPass, Other.RenderPass);
			std::swap(Attachments, Other.Attachments);
			std::swap(Framebuffer, Other.Framebuffer);
			std::swap(Size, Other.Size);

			return *this;
		}

		Vec2ui VulkanRenderTarget::GetSize() const
		{
			return Size;
		}

		uint32 VulkanRenderTarget::GetImageCount() const
		{
			return (uint32)Attachments.size();
		}

		VkFramebuffer VulkanRenderTarget::GetFramebuffer() const
		{
			return Framebuffer;
		}
	}
}
