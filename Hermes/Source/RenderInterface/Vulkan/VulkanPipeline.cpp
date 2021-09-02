#include "VulkanPipeline.h"

#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanRenderPass.h"
#include "RenderInterface/Vulkan/VulkanShader.h"
#include "RenderInterface/Vulkan/VulkanCommonTypes.h"

namespace Hermes
{
	namespace Vulkan
	{
		static VkShaderStageFlagBits ShaderTypeToVkShaderStage(RenderInterface::ShaderType Type)
		{
			switch (Type)
			{
			case RenderInterface::ShaderType::VertexShader:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case RenderInterface::ShaderType::FragmentShader:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			default:
				HERMES_ASSERT(false);
				return (VkShaderStageFlagBits)0;
			}
		}

		static VkPrimitiveTopology TopologyTypeToVkPrimitiveTopology(RenderInterface::TopologyType Type)
		{
			switch (Type)
			{
			case RenderInterface::TopologyType::PointList:
				return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case RenderInterface::TopologyType::LineList:
				return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case RenderInterface::TopologyType::LineStrip:
				return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case RenderInterface::TopologyType::TriangleList:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case RenderInterface::TopologyType::TriangleStrip:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case RenderInterface::TopologyType::TriangleFan:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			default: 
				HERMES_ASSERT(false);
				return (VkPrimitiveTopology)0;
			}
		}

		static VkCullModeFlags CullModeToVkCullMode(RenderInterface::CullMode Mode)
		{
			switch (Mode)
			{
			case RenderInterface::CullMode::Front:
				return VK_CULL_MODE_FRONT_BIT;
			case RenderInterface::CullMode::Back:
				return VK_CULL_MODE_BACK_BIT;
			case RenderInterface::CullMode::Both:
				return VK_CULL_MODE_FRONT_AND_BACK;
			default:
				HERMES_ASSERT(false);
				return (VkCullModeFlags)0;
			}
		}

		static VkPolygonMode FillModeToVkPolygonMode(RenderInterface::FillMode Mode)
		{
			switch (Mode)
			{
			case RenderInterface::FillMode::Fill:
				return VK_POLYGON_MODE_FILL;
			case RenderInterface::FillMode::Lines:
				return VK_POLYGON_MODE_LINE;
			case RenderInterface::FillMode::Points:
				return VK_POLYGON_MODE_POINT;
			default:
				HERMES_ASSERT(false);
				return (VkPolygonMode)0;
			}
		}

		static VkFrontFace FaceDirectionToVkFrontFace(RenderInterface::FaceDirection Direction)
		{
			switch (Direction)
			{
			case RenderInterface::FaceDirection::Clockwise:
				return VK_FRONT_FACE_CLOCKWISE;
			case RenderInterface::FaceDirection::CounterClockwise:
				return VK_FRONT_FACE_COUNTER_CLOCKWISE;
			default:
				HERMES_ASSERT(false);
				return (VkFrontFace)0;
			}
		}

		static VkCompareOp ComparisonOperatorToVkCompareOp(RenderInterface::ComparisonOperator Operator)
		{
			switch (Operator)
			{
			case RenderInterface::ComparisonOperator::AlwaysFail:
				return VK_COMPARE_OP_NEVER;
			case RenderInterface::ComparisonOperator::AlwaysSucceed:
				return VK_COMPARE_OP_ALWAYS;
			case RenderInterface::ComparisonOperator::NotEqual:
				return VK_COMPARE_OP_NOT_EQUAL;
			case RenderInterface::ComparisonOperator::Less:
				return VK_COMPARE_OP_LESS;
			case RenderInterface::ComparisonOperator::LessOrEqual:
				return VK_COMPARE_OP_LESS_OR_EQUAL;
			case RenderInterface::ComparisonOperator::Equal:
				return VK_COMPARE_OP_EQUAL;
			case RenderInterface::ComparisonOperator::GreaterOrEqual:
				return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case RenderInterface::ComparisonOperator::Greater:
				return VK_COMPARE_OP_GREATER;
			default:
				HERMES_ASSERT(false);
				return (VkCompareOp)0;
			}
		}
		
		VulkanPipeline::VulkanPipeline(std::shared_ptr<VulkanDevice> InDevice, std::shared_ptr<RenderInterface::RenderPass> InRenderPass, const RenderInterface::PipelineDescription& Description)
			: Device(std::move(InDevice))
			, RenderPass(std::reinterpret_pointer_cast<VulkanRenderPass>(InRenderPass)) // TODO : seems like a very dirty hack, maybe there's something better for this?
			, Pipeline(VK_NULL_HANDLE)
			, Layout(VK_NULL_HANDLE)
		{
			VkPipelineLayoutCreateInfo LayoutCreateInfo = {};
			LayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			// TODO : other fields
			VK_CHECK_RESULT(vkCreatePipelineLayout(Device->GetDevice(), &LayoutCreateInfo, GVulkanAllocator, &Layout));

			std::vector<VkPipelineShaderStageCreateInfo> ShaderCreateInfos(Description.ShaderStages.size());
			for (size_t Index = 0; Index < Description.ShaderStages.size(); Index++)
			{
				ShaderCreateInfos[Index].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				ShaderCreateInfos[Index].module = ((VulkanShader*)Description.ShaderStages[Index].get())->GetShader();
				ShaderCreateInfos[Index].pName = "main"; // TODO : allow user to choice or decide using reflection
				ShaderCreateInfos[Index].stage = ShaderTypeToVkShaderStage(Description.ShaderStages[Index]->GetType());
			}

			std::vector<VkVertexInputAttributeDescription> InputAttributes;
			InputAttributes.reserve(Description.VertexInput.VertexAttributes.size());
			std::vector<VkVertexInputBindingDescription> InputBindings;
			InputBindings.reserve(Description.VertexInput.VertexBindings.size());
			for (const auto& Attribute : Description.VertexInput.VertexAttributes)
			{
				VkVertexInputAttributeDescription NewAttribute = {};
				NewAttribute.location = Attribute.Location;
				NewAttribute.offset = Attribute.Offset;
				NewAttribute.binding = Attribute.BindingIndex;
				NewAttribute.format = DataFormatToVkFormat(Attribute.Format);
				InputAttributes.push_back(NewAttribute);
			}
			for (const auto& Binding : Description.VertexInput.VertexBindings)
			{
				VkVertexInputBindingDescription NewBinding = {};
				NewBinding.binding = Binding.Index;
				NewBinding.inputRate = Binding.IsPerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
				NewBinding.stride = Binding.Stride;
				InputBindings.push_back(NewBinding);
			}

			VkPipelineVertexInputStateCreateInfo VertexInputCreateInfo = {};
			VertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			VertexInputCreateInfo.vertexAttributeDescriptionCount = (uint32)InputAttributes.size();
			VertexInputCreateInfo.pVertexAttributeDescriptions = InputAttributes.data();
			VertexInputCreateInfo.vertexBindingDescriptionCount = (uint32)InputBindings.size();
			VertexInputCreateInfo.pVertexBindingDescriptions = InputBindings.data();

			VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo = {};
			InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			InputAssemblyCreateInfo.primitiveRestartEnable = false;
			InputAssemblyCreateInfo.topology = TopologyTypeToVkPrimitiveTopology(Description.InputAssembler.Topology);

			VkPipelineViewportStateCreateInfo ViewportCreateInfo = {};
			ViewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			ViewportCreateInfo.viewportCount = 1;
			VkViewport Viewport = {};
			Viewport.x        = (float)Description.Viewport.Origin.X;
			Viewport.y        = (float)Description.Viewport.Origin.Y;
			Viewport.width    = (float)Description.Viewport.Dimensions.X;
			Viewport.height   = (float)Description.Viewport.Dimensions.Y;
			Viewport.minDepth = 0.0f;
			Viewport.maxDepth = 1.0f;
			ViewportCreateInfo.pViewports = &Viewport;
			ViewportCreateInfo.scissorCount = 1;
			VkRect2D Scissor;
			Scissor.offset = { 0, 0 };
			Scissor.extent = { Description.Viewport.Dimensions.X, Description.Viewport.Dimensions.Y };
			ViewportCreateInfo.pScissors = &Scissor;

			VkPipelineRasterizationStateCreateInfo RasterizationCreateInfo = {};
			RasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			RasterizationCreateInfo.cullMode = CullModeToVkCullMode(Description.Rasterizer.Cull);
			RasterizationCreateInfo.polygonMode = FillModeToVkPolygonMode(Description.Rasterizer.Fill);
			RasterizationCreateInfo.frontFace = FaceDirectionToVkFrontFace(Description.Rasterizer.Direction);
			RasterizationCreateInfo.lineWidth = 1.0f;

			VkPipelineMultisampleStateCreateInfo MultisampleCreateInfo = {};
			MultisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			MultisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo = {};
			DepthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			DepthStencilCreateInfo.depthTestEnable = Description.DepthStencilStage.IsDepthTestEnabled;
			DepthStencilCreateInfo.depthWriteEnable = Description.DepthStencilStage.IsDepthWriteEnabled;
			DepthStencilCreateInfo.depthCompareOp = ComparisonOperatorToVkCompareOp(Description.DepthStencilStage.ComparisonMode);
			DepthStencilCreateInfo.depthBoundsTestEnable = false;
			DepthStencilCreateInfo.stencilTestEnable = false; // TODO : implement

			VkPipelineColorBlendStateCreateInfo ColorBlendCreateInfo = {};
			std::vector<VkPipelineColorBlendAttachmentState> AttachmentBlendStates;
			AttachmentBlendStates.reserve(RenderPass->GetColorAttachmentCount());
			for (uint32 AttachmentIndex = 0; AttachmentIndex < RenderPass->GetColorAttachmentCount(); AttachmentIndex++)
			{
				VkPipelineColorBlendAttachmentState NewAttachmentBlendState = {};
				NewAttachmentBlendState.blendEnable = false;
				NewAttachmentBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				AttachmentBlendStates.push_back(NewAttachmentBlendState);
			}
			ColorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			ColorBlendCreateInfo.attachmentCount = (uint32)AttachmentBlendStates.size();
			ColorBlendCreateInfo.pAttachments = AttachmentBlendStates.data();
			
			VkGraphicsPipelineCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			CreateInfo.stageCount = (uint32)ShaderCreateInfos.size();
			CreateInfo.pStages = ShaderCreateInfos.data();
			CreateInfo.pVertexInputState = &VertexInputCreateInfo;
			CreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;
			CreateInfo.pTessellationState = nullptr;
			CreateInfo.pViewportState = &ViewportCreateInfo;
			CreateInfo.pRasterizationState = &RasterizationCreateInfo;
			CreateInfo.pMultisampleState = &MultisampleCreateInfo;
			CreateInfo.pDepthStencilState = &DepthStencilCreateInfo;
			CreateInfo.pColorBlendState = &ColorBlendCreateInfo;
			CreateInfo.pDynamicState = nullptr; // TODO : implement
			CreateInfo.layout = Layout;
			CreateInfo.renderPass = RenderPass->GetRenderPass();
			CreateInfo.subpass = 0;
			CreateInfo.basePipelineHandle = VK_NULL_HANDLE;
			CreateInfo.basePipelineIndex = -1;

			vkCreateGraphicsPipelines(Device->GetDevice(), VK_NULL_HANDLE, 1, &CreateInfo, GVulkanAllocator, &Pipeline);
		}

		VulkanPipeline::~VulkanPipeline()
		{
			vkDestroyPipelineLayout(Device->GetDevice(), Layout, GVulkanAllocator);
			vkDestroyPipeline(Device->GetDevice(), Pipeline, GVulkanAllocator);
		}

		VulkanPipeline::VulkanPipeline(VulkanPipeline&& Other)
		{
			*this = std::move(Other);
		}

		VulkanPipeline& VulkanPipeline::operator=(VulkanPipeline&& Other)
		{
			std::swap(Pipeline, Other.Pipeline);
			std::swap(RenderPass, Other.RenderPass);
			std::swap(Layout, Other.Layout);
			std::swap(Device, Other.Device);
			return *this;
		}

		VkPipeline VulkanPipeline::GetPipeline() const
		{
			return Pipeline;
		}
	}
}
