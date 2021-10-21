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

		VulkanRenderPass::VulkanRenderPass(std::shared_ptr<VulkanDevice> InDevice, const RenderInterface::RenderPassDescription& Description)
			: Device(std::move(InDevice))
			, RenderPass(VK_NULL_HANDLE)
			, ColorAttachmentCount(0)
		{
			std::vector<VkAttachmentDescription> VulkanAttachments;
			VulkanAttachments.reserve(Description.InputAttachments.size() + Description.ColorAttachments.size() + (size_t)Description.DepthAttachment.has_value());
			auto FillVkAttachment = [](const RenderInterface::RenderPassAttachment& Attachment)
			{
				VkAttachmentDescription NewAttachment = {};
				NewAttachment.format = DataFormatToVkFormat(Attachment.Format);
				NewAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				NewAttachment.loadOp = LoadOpToVkAttachmentLoadOp(Attachment.LoadOp);
				NewAttachment.storeOp = StoreOpToVkAttachmentStoreOp(Attachment.StoreOp);
				NewAttachment.stencilLoadOp = LoadOpToVkAttachmentLoadOp(Attachment.StencilLoadOp);
				NewAttachment.stencilStoreOp = StoreOpToVkAttachmentStoreOp(Attachment.StencilStoreOp);
				NewAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				NewAttachment.finalLayout = ImageLayoutToVkImageLayout(Attachment.LayoutAtEnd);
				return NewAttachment;
			};
			// NOTE : keep in following order, VkSubpassDescription generation depends on it
			for (const auto& Attachment : Description.InputAttachments)
				VulkanAttachments.push_back(FillVkAttachment(Attachment));
			for (const auto& Attachment : Description.ColorAttachments)
				VulkanAttachments.push_back(FillVkAttachment(Attachment));
			if (Description.DepthAttachment.has_value())
				VulkanAttachments.push_back(FillVkAttachment(Description.DepthAttachment.value()));

			VkSubpassDescription Subpass = {};
			Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			std::vector<VkAttachmentReference> InputAttachmentReferences;
			InputAttachmentReferences.reserve(Description.InputAttachments.size());
			std::vector<VkAttachmentReference> ColorAttachmentReferences;
			ColorAttachmentReferences.reserve(Description.ColorAttachments.size());
			VkAttachmentReference DepthAttachment;
			uint32 Offset = 0;
			auto FillAttachmentReference = [](const RenderInterface::RenderPassAttachment& Attachment, uint32 Index)
			{
				VkAttachmentReference Result;
				Result.attachment = (uint32)(Index);
				Result.layout = ImageLayoutToVkImageLayout(Attachment.LayoutAtStart);
				return Result;
			};
			auto FillAttachmentReferenceVector = [&Offset, FillAttachmentReference](const std::vector<RenderInterface::RenderPassAttachment>& From, std::vector<VkAttachmentReference>& To)
			{
				for (uint32 Index = 0; Index < (uint32)From.size(); Index++)
					To.push_back(FillAttachmentReference(From[Index], Index + Offset));
				Offset += (uint32)From.size();
			};
			FillAttachmentReferenceVector(Description.InputAttachments, InputAttachmentReferences);
			Subpass.pInputAttachments = InputAttachmentReferences.data();
			Subpass.inputAttachmentCount = (uint32)InputAttachmentReferences.size();
			FillAttachmentReferenceVector(Description.ColorAttachments, ColorAttachmentReferences);
			Subpass.pColorAttachments = ColorAttachmentReferences.data();
			Subpass.colorAttachmentCount = (uint32)ColorAttachmentReferences.size();
			if (Description.DepthAttachment.has_value())
			{
				DepthAttachment = FillAttachmentReference(Description.DepthAttachment.value(), Offset);
				Subpass.pDepthStencilAttachment = &DepthAttachment;
			}
			
			VkRenderPassCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			CreateInfo.attachmentCount = (uint32)VulkanAttachments.size();
			CreateInfo.pAttachments = VulkanAttachments.data();
			CreateInfo.subpassCount = 1;
			CreateInfo.pSubpasses = &Subpass;
			CreateInfo.dependencyCount = 0;
			CreateInfo.pDependencies = nullptr;

			VK_CHECK_RESULT(vkCreateRenderPass(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &RenderPass));
			ColorAttachmentCount = (uint32)Description.ColorAttachments.size();
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
