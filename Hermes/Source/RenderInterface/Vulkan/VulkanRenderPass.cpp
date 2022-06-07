#include "VulkanRenderPass.h"

#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanCommonTypes.h"

namespace Hermes
{
	namespace Vulkan
	{
		static VkAttachmentLoadOp LoadOpToVkAttachmentLoadOp(RenderInterface::AttachmentLoadOp Op)
		{
			switch (Op)
			{
			case RenderInterface::AttachmentLoadOp::Clear:
				return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case RenderInterface::AttachmentLoadOp::Load:
				return VK_ATTACHMENT_LOAD_OP_LOAD;
			case RenderInterface::AttachmentLoadOp::Undefined:
				return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			default:
				HERMES_ASSERT(false);
				return (VkAttachmentLoadOp)0;
			}
		}

		static VkAttachmentStoreOp StoreOpToVkAttachmentStoreOp(RenderInterface::AttachmentStoreOp Op)
		{
			switch (Op)
			{
			case RenderInterface::AttachmentStoreOp::Store:
				return VK_ATTACHMENT_STORE_OP_STORE;
			case RenderInterface::AttachmentStoreOp::Undefined:
				return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			default:
				HERMES_ASSERT(false);
				return (VkAttachmentStoreOp)0;
			}
		}

		VulkanRenderPass::VulkanRenderPass(
			std::shared_ptr<const VulkanDevice> InDevice, const std::vector<RenderInterface::RenderPassAttachment>& Attachments)
			: Device(std::move(InDevice))
			, RenderPass(VK_NULL_HANDLE)
			, ColorAttachmentCount(0)
		{
			// Check if we have only one depth attachment and calculate number
			// of color and input attachments
			bool DepthAttachmentWasFound = false;
			for (const auto& Attachment : Attachments)
			{
				if (Attachment.Type == RenderInterface::AttachmentType::DepthStencil)
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
				else if (Attachment.Type == RenderInterface::AttachmentType::Color)
					ColorAttachmentCount++;
			}

			std::vector<VkAttachmentDescription> VulkanAttachments;
			VulkanAttachments.reserve(Attachments.size());
			auto FillVkAttachment = [](const RenderInterface::RenderPassAttachment& Attachment)
			{
				VkAttachmentDescription NewAttachment = {};
				NewAttachment.format = DataFormatToVkFormat(Attachment.Format);
				NewAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				NewAttachment.loadOp = LoadOpToVkAttachmentLoadOp(Attachment.LoadOp);
				NewAttachment.storeOp = StoreOpToVkAttachmentStoreOp(Attachment.StoreOp);
				NewAttachment.stencilLoadOp = LoadOpToVkAttachmentLoadOp(Attachment.StencilLoadOp);
				NewAttachment.stencilStoreOp = StoreOpToVkAttachmentStoreOp(Attachment.StencilStoreOp);
				NewAttachment.initialLayout = ImageLayoutToVkImageLayout(Attachment.LayoutAtStart);
				NewAttachment.finalLayout = ImageLayoutToVkImageLayout(Attachment.LayoutAtEnd);
				return NewAttachment;
			};

			std::vector<VkAttachmentReference> ColorAttachmentReferences, InputAttachmentReferences;
			VkAttachmentReference DepthAttachment;
			for (auto It = Attachments.begin(); It != Attachments.end(); ++It)
			{
				const auto& Attachment = *It;
				VulkanAttachments.push_back(FillVkAttachment(Attachment));

				VkAttachmentReference Reference = {};
				Reference.attachment = static_cast<uint32>(std::distance(Attachments.begin(), It));
				Reference.layout = ImageLayoutToVkImageLayout(Attachment.LayoutAtStart);
				switch (Attachment.Type)
				{
				case RenderInterface::AttachmentType::Color:
					ColorAttachmentReferences.push_back(Reference);
					break;
				case RenderInterface::AttachmentType::Input:
					InputAttachmentReferences.push_back(Reference);
					break;
				case RenderInterface::AttachmentType::DepthStencil:
					DepthAttachment = Reference;
					break;
				default:
					HERMES_ASSERT_LOG(false, L"Unknown render pass attachment type.");
					break;
				}
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
			CreateInfo.attachmentCount = static_cast<uint32>(VulkanAttachments.size());
			CreateInfo.pAttachments = VulkanAttachments.data();
			CreateInfo.subpassCount = 1;
			CreateInfo.pSubpasses = &Subpass;
			CreateInfo.dependencyCount = 0;
			CreateInfo.pDependencies = nullptr;

			VK_CHECK_RESULT(vkCreateRenderPass(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &RenderPass));
		}

		VulkanRenderPass::~VulkanRenderPass()
		{
			vkDestroyRenderPass(Device->GetDevice(), RenderPass, GVulkanAllocator);
		}

		VulkanRenderPass::VulkanRenderPass(VulkanRenderPass&& Other)
		{
			*this = std::move(Other);
		}

		VulkanRenderPass& VulkanRenderPass::operator=(VulkanRenderPass&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(RenderPass, Other.RenderPass);
			std::swap(ColorAttachmentCount, Other.ColorAttachmentCount);
			return *this;
		}

		uint32 VulkanRenderPass::GetColorAttachmentCount() const
		{
			return ColorAttachmentCount;
		}
	}
}
