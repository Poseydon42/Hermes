#include "Framebuffer.h"

#include "Vulkan/Image.h"
#include "Vulkan/RenderPass.h"

namespace Hermes::Vulkan
{
	Framebuffer::Framebuffer(std::shared_ptr<Device::VkDeviceHolder> InDevice, const RenderPass& InRenderPass,
	                         const std::vector<const ImageView*>& InAttachments, Vec2ui InDimensions)
		: Device(std::move(InDevice))
		, ImageCount(static_cast<uint32>(InAttachments.size()))
		, Dimensions(InDimensions)
	{
		std::vector<VkImageView> AttachmentViews(InAttachments.size(), VK_NULL_HANDLE);
		for (size_t Index = 0; Index < AttachmentViews.size(); Index++)
		{
			AttachmentViews[Index] = InAttachments[Index]->GetImageView();
		}

		VkFramebufferCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		CreateInfo.attachmentCount = static_cast<uint32>(AttachmentViews.size());
		CreateInfo.pAttachments = AttachmentViews.data();
		CreateInfo.width = Dimensions.X;
		CreateInfo.height = Dimensions.Y;
		CreateInfo.layers = 1;
		CreateInfo.renderPass = InRenderPass.GetRenderPass();

		VK_CHECK_RESULT(vkCreateFramebuffer(Device->Device, &CreateInfo, GVulkanAllocator, &Handle));
	}

	Framebuffer::~Framebuffer()
	{
		vkDestroyFramebuffer(Device->Device, Handle, GVulkanAllocator);
	}
	
	Vec2ui Framebuffer::GetDimensions() const
	{
		return Dimensions;
	}

	uint32 Framebuffer::GetImageCount() const
	{
		return ImageCount;
	}

	VkFramebuffer Framebuffer::GetFramebuffer() const
	{
		return Handle;
	}
}
