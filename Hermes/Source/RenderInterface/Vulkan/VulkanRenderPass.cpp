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

		static VkPipelineStageFlags PipelineStageToVkPipelineStageFlags(RenderInterface::PipelineStage Stage)
		{
			VkPipelineStageFlags Result = {};
			
#define CORRESPONDING_BIT(SrcBit, DestBit) \
			if ((bool)(Stage & RenderInterface::PipelineStage::SrcBit)) \
			{ \
				Result |= (DestBit); \
				Stage &= ~RenderInterface::PipelineStage::SrcBit; \
			}

			CORRESPONDING_BIT(TopOfPipe, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
			CORRESPONDING_BIT(DrawIndirect, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
			CORRESPONDING_BIT(VertexInput, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
			CORRESPONDING_BIT(VertexShader, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
			CORRESPONDING_BIT(TessellationControlShader, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT);
			CORRESPONDING_BIT(TessellationEvaluationShader, VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT);
			CORRESPONDING_BIT(GeometryShader, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT);
			CORRESPONDING_BIT(FragmentShader, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			CORRESPONDING_BIT(EarlyFragmentTests, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
			CORRESPONDING_BIT(LateFragmentTests, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
			CORRESPONDING_BIT(ColorAttachmentOutput, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			CORRESPONDING_BIT(Transfer, VK_PIPELINE_STAGE_TRANSFER_BIT);
			CORRESPONDING_BIT(BottomOfPipe, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
			CORRESPONDING_BIT(Host, VK_PIPELINE_STAGE_HOST_BIT);
			CORRESPONDING_BIT(AllCommands, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
			
#undef CORRESPONDING_BIT
			// This is a bit dirty hack: when we check Stage against each bit, we set this bit to 0;
			// so, in case we forget to check some bits and they were set to 1, Stage won't be 0 anymore and assert will fail immediately
			HERMES_ASSERT((uint64)Stage == 0);
			return Result;
		}

		static VkAccessFlags AccessTypeToVkAccessFlags(RenderInterface::AccessType Type)
		{
				VkAccessFlags Result = {};
			
#define CORRESPONDING_BIT(SrcBit, DestBit) \
			if ((bool)(Type & RenderInterface::AccessType::SrcBit)) \
			{ \
				Result |= (DestBit); \
				Type &= ~RenderInterface::AccessType::SrcBit; \
			}
			
			CORRESPONDING_BIT(IndirectCommandRead, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
			CORRESPONDING_BIT(IndexRead, VK_ACCESS_INDEX_READ_BIT);
			CORRESPONDING_BIT(VertexAttributeRead, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
			CORRESPONDING_BIT(UniformRead, VK_ACCESS_UNIFORM_READ_BIT);
			CORRESPONDING_BIT(InputAttachmentRead, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT);
			CORRESPONDING_BIT(ShaderRead, VK_ACCESS_SHADER_READ_BIT);
			CORRESPONDING_BIT(ShaderWrite, VK_ACCESS_SHADER_WRITE_BIT);
			CORRESPONDING_BIT(ColorAttachmentRead, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);
			CORRESPONDING_BIT(ColorAttachmentWrite, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
			CORRESPONDING_BIT(DepthStencilAttachmentRead, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT);
			CORRESPONDING_BIT(DepthStencilAttachmentWrite, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
			CORRESPONDING_BIT(TransferRead, VK_ACCESS_TRANSFER_READ_BIT);
			CORRESPONDING_BIT(TransferWrite, VK_ACCESS_TRANSFER_WRITE_BIT);
			CORRESPONDING_BIT(HostRead, VK_ACCESS_HOST_READ_BIT);
			CORRESPONDING_BIT(HostWrite, VK_ACCESS_HOST_WRITE_BIT);
			CORRESPONDING_BIT(MemoryRead, VK_ACCESS_MEMORY_READ_BIT)
			CORRESPONDING_BIT(MemoryWrite, VK_ACCESS_MEMORY_WRITE_BIT);
			
#undef CORRESPONDING_BIT
			// This is a bit dirty hack: when we check Stage against each bit, we set this bit to 0;
			// so, in case we forget to check some bits and they were set to 1, Type won't be 0 anymore and assert will fail immediately
			HERMES_ASSERT((uint64)Type == 0);
			return Result;
		}

		VulkanRenderPass::VulkanRenderPass(std::shared_ptr<VulkanDevice> InDevice, const RenderInterface::RenderPassDescription& Description)
			: Device(std::move(InDevice))
			, SubpassNumber((uint32)Description.Subpasses.size())
			, ColorAttachmentNumbers((uint32)Description.Subpasses.size(), 0)
			, RenderPass(VK_NULL_HANDLE)
		{
			std::vector<VkAttachmentDescription> VulkanAttachments;
			VulkanAttachments.reserve(Description.Attachments.size());
			for (const auto& Attachment : Description.Attachments)
			{
				VkAttachmentDescription NewAttachment = {};
				NewAttachment.format = (VkFormat)Attachment.Format; // TODO : replace with valid enum conversion function when we'd have one
				NewAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				NewAttachment.loadOp = LoadOpToVkAttachmentLoadOp(Attachment.LoadOp);
				NewAttachment.storeOp = StoreOpToVkAttachmentStoreOp(Attachment.StoreOp);
				NewAttachment.stencilLoadOp = LoadOpToVkAttachmentLoadOp(Attachment.StencilLoadOp);
				NewAttachment.stencilStoreOp = StoreOpToVkAttachmentStoreOp(Attachment.StencilStoreOp);
				NewAttachment.initialLayout = ImageLayoutToVkImageLayout(Attachment.LayoutBeforeBegin);
				NewAttachment.finalLayout = ImageLayoutToVkImageLayout(Attachment.LayoutAtEnd);
				VulkanAttachments.push_back(NewAttachment);
			}

			std::vector<VkSubpassDescription> VulkanSubpasses;
			VulkanSubpasses.reserve(Description.Subpasses.size());
			// This is a bit dirty hack - we need all VkAttachmentReference instances to be 'in scope' until the end of this function,
			// so we may not declare this vector inside for loop that iterates over subpasses. Instead, we create vector of all attachment
			// references ever used, and in VkSubpassDescription set pointer to one of its elements. This way, our vector would be destroyed
			// when we will exit the constructor scope, which obviously would be after vkCreateRenderPass is being called
			std::vector<VkAttachmentReference> VulkanAttachmentReferences;
			for (const auto& Subpass : Description.Subpasses)
			{
				VkSubpassDescription NewSubpass = {};
				NewSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // TODO : compute pipelines
				
				for (const auto& Attachment : Subpass.InputAttachments)
				{
					VkAttachmentReference NewReference = {};
					NewReference.attachment = Attachment.Index;
					NewReference.layout = ImageLayoutToVkImageLayout(Attachment.Layout);
					VulkanAttachmentReferences.push_back(NewReference);
				}
				NewSubpass.inputAttachmentCount = (uint32)Subpass.InputAttachments.size();
				if (NewSubpass.inputAttachmentCount > 0)
					NewSubpass.pInputAttachments = &*(VulkanAttachmentReferences.end() - NewSubpass.inputAttachmentCount);

				for (const auto& Attachment : Subpass.ColorAttachments)
				{
					VkAttachmentReference NewReference = {};
					NewReference.attachment = Attachment.Index;
					NewReference.layout = ImageLayoutToVkImageLayout(Attachment.Layout);
					VulkanAttachmentReferences.push_back(NewReference);
				}
				NewSubpass.colorAttachmentCount = (uint32)Subpass.ColorAttachments.size();
				if (NewSubpass.colorAttachmentCount > 0)
					NewSubpass.pColorAttachments = &*(VulkanAttachmentReferences.end() - NewSubpass.colorAttachmentCount);
				
				if (Subpass.IsDepthStencilAttachmentUsed)
				{
					VkAttachmentReference DepthStencilAttachmentReference = {};
					DepthStencilAttachmentReference.attachment = Subpass.DepthStencilAttachment.Index;
					DepthStencilAttachmentReference.layout = ImageLayoutToVkImageLayout(Subpass.DepthStencilAttachment.Layout);
					VulkanAttachmentReferences.push_back(DepthStencilAttachmentReference);
					NewSubpass.pDepthStencilAttachment = &VulkanAttachmentReferences.back();
				}
				else
				{
					NewSubpass.pDepthStencilAttachment = nullptr;
				}

				NewSubpass.pPreserveAttachments = Subpass.PreservedAttachmentsIndices.data();
				NewSubpass.preserveAttachmentCount = (uint32)Subpass.PreservedAttachmentsIndices.size();
				
				VulkanSubpasses.push_back(NewSubpass);
			}

			std::vector<VkSubpassDependency> VulkanSubpassDependencies;
			VulkanSubpassDependencies.reserve(Description.Dependencies.size());
			for (const auto& Dependency : Description.Dependencies)
			{
				VkSubpassDependency NewDependency = {};
				NewDependency.srcSubpass = Dependency.SourceSubpassIndex;
				NewDependency.dstSubpass = Dependency.DestinationSubpassIndex;
				NewDependency.srcStageMask = PipelineStageToVkPipelineStageFlags(Dependency.SourceStage);
				NewDependency.dstStageMask = PipelineStageToVkPipelineStageFlags(Dependency.DestinationStage);
				NewDependency.srcAccessMask = AccessTypeToVkAccessFlags(Dependency.SourceAccessType);
				NewDependency.dstAccessMask = AccessTypeToVkAccessFlags(Dependency.DestinationAccessType);
				VulkanSubpassDependencies.push_back(NewDependency);
			}
			
			VkRenderPassCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			CreateInfo.attachmentCount = (uint32)VulkanAttachments.size();
			CreateInfo.pAttachments = VulkanAttachments.data();
			CreateInfo.subpassCount = (uint32)VulkanSubpasses.size();
			CreateInfo.pSubpasses = VulkanSubpasses.data();
			CreateInfo.dependencyCount = (uint32)VulkanSubpassDependencies.size();
			CreateInfo.pDependencies = VulkanSubpassDependencies.data();

			VK_CHECK_RESULT(vkCreateRenderPass(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &RenderPass));

			for (size_t Index = 0; Index < Description.Subpasses.size(); Index++)
				ColorAttachmentNumbers[Index] = (uint32)Description.Subpasses[Index].ColorAttachments.size();
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
			return *this;
		}
	}
}
