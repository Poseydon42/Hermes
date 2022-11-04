#include "RenderPass.h"

#include "Vulkan/Image.h"

namespace Hermes::Vulkan
{
	RenderPass::RenderPass(std::shared_ptr<Device::VkDeviceHolder> InDevice,
	                       const std::vector<std::pair<VkAttachmentDescription, AttachmentType>>& Attachments)
		: Device(std::move(InDevice))
	{
		// Check if we have only one depth attachment and calculate number
		// of color and input attachments
		bool DepthAttachmentWasFound = false;
		for (const auto& Attachment : Attachments)
		{
			if (Attachment.second == AttachmentType::DepthStencil)
			{
				if (DepthAttachmentWasFound)
				{
					HERMES_ASSERT_LOG(false, L"Trying to create render pass with more than one depth attachment");
				}
				else
				{
					DepthAttachmentWasFound = true;
				}
			}
			else if (Attachment.second == AttachmentType::Color)
				ColorAttachmentCount++;
		}

		std::vector<VkAttachmentReference> ColorAttachmentReferences, InputAttachmentReferences;
		VkAttachmentReference DepthAttachment;
		for (auto It = Attachments.begin(); It != Attachments.end(); ++It)
		{
			const auto& Attachment = *It;

			VkAttachmentReference Reference = {};
			Reference.attachment = static_cast<uint32>(std::distance(Attachments.begin(), It));
			Reference.layout = Attachment.first.initialLayout;
			switch (Attachment.second)
			{
			case AttachmentType::Color:
				ColorAttachmentReferences.push_back(Reference);
				break;
			case AttachmentType::Input:
				InputAttachmentReferences.push_back(Reference);
				break;
			case AttachmentType::DepthStencil:
				DepthAttachment = Reference;
			}
		}

		std::vector<VkAttachmentDescription> VkAttachments(Attachments.size());
		for (size_t Index = 0; Index < Attachments.size(); Index++)
		{
			VkAttachments[Index] = Attachments[Index].first;
		}

		VkSubpassDescription Subpass = {};
		Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		Subpass.pInputAttachments = InputAttachmentReferences.data();
		Subpass.inputAttachmentCount = static_cast<uint32>(InputAttachmentReferences.size());
		Subpass.pColorAttachments = ColorAttachmentReferences.data();
		Subpass.colorAttachmentCount = static_cast<uint32>(ColorAttachmentReferences.size());
		if (DepthAttachmentWasFound)
			Subpass.pDepthStencilAttachment = &DepthAttachment;

		VkRenderPassCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		CreateInfo.attachmentCount = static_cast<uint32>(Attachments.size());
		CreateInfo.pAttachments = VkAttachments.data();
		CreateInfo.subpassCount = 1;
		CreateInfo.pSubpasses = &Subpass;
		CreateInfo.dependencyCount = 0;
		CreateInfo.pDependencies = nullptr;

		VK_CHECK_RESULT(vkCreateRenderPass(Device->Device, &CreateInfo, GVulkanAllocator, &Handle));
	}

	RenderPass::~RenderPass()
	{
		vkDestroyRenderPass(Device->Device, Handle, GVulkanAllocator);
	}

	uint32 RenderPass::GetColorAttachmentCount() const
	{
		return ColorAttachmentCount;
	}

	VkRenderPass RenderPass::GetRenderPass() const
	{
		return Handle;
	}
}
