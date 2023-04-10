﻿#include "Pipeline.h"

#include "Vulkan/Descriptor.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Shader.h"

namespace Hermes::Vulkan
{
	Pipeline::Pipeline(std::shared_ptr<Device::VkDeviceHolder> InDevice, const RenderPass& RenderPass,
	                   const PipelineDescription& Description)
		: Pipeline(std::move(InDevice), Description, &RenderPass, {}, std::nullopt)
	{
		
	}

	Pipeline::Pipeline(std::shared_ptr<Device::VkDeviceHolder> InDevice, const PipelineDescription& Description, std::span<const VkFormat> ColorAttachmentFormats, std::optional<VkFormat> DepthAttachmentFormat)
		: Pipeline(std::move(InDevice), Description, nullptr, ColorAttachmentFormats, DepthAttachmentFormat)
	{
	}

	Pipeline::~Pipeline()
	{
		vkDestroyPipelineLayout(Device->Device, Layout, GVulkanAllocator);
		vkDestroyPipeline(Device->Device, Handle, GVulkanAllocator);
	}

	VkPipeline Pipeline::GetPipeline() const
	{
		return Handle;
	}

	VkPipelineLayout Pipeline::GetPipelineLayout() const
	{
		return Layout;
	}

	Pipeline::Pipeline(std::shared_ptr<Device::VkDeviceHolder> InDevice, const PipelineDescription& Description, const RenderPass* RenderPass, std::span<const VkFormat> ColorAttachmentFormats, std::optional<VkFormat> DepthAttachmentFormat)
		: Device(std::move(InDevice))
	{
		VkPipelineLayoutCreateInfo LayoutCreateInfo = {};
		LayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> Layouts;
		Layouts.reserve(Description.DescriptorSetLayouts.size());
		for (const auto& CurrentLayout : Description.DescriptorSetLayouts)
		{
			Layouts.push_back(CurrentLayout->GetDescriptorSetLayout());
		}

		LayoutCreateInfo.setLayoutCount = static_cast<uint32>(Layouts.size());
		LayoutCreateInfo.pSetLayouts = Layouts.data();
		LayoutCreateInfo.pushConstantRangeCount = static_cast<uint32>(Description.PushConstants.size());
		LayoutCreateInfo.pPushConstantRanges = Description.PushConstants.data();
		VK_CHECK_RESULT(vkCreatePipelineLayout(Device->Device, &LayoutCreateInfo, GVulkanAllocator, &Layout));

		std::vector<VkPipelineShaderStageCreateInfo> ShaderCreateInfos(Description.ShaderStages.size());
		for (size_t Index = 0; Index < Description.ShaderStages.size(); Index++)
		{
			ShaderCreateInfos[Index].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			ShaderCreateInfos[Index].module = Description.ShaderStages[Index]->GetShader();
			ShaderCreateInfos[Index].pName = "main"; // TODO : allow user to choice or decide using reflection
			ShaderCreateInfos[Index].stage = Description.ShaderStages[Index]->GetType();
		}

		VkPipelineVertexInputStateCreateInfo VertexInputCreateInfo = {};
		VertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32>(Description.VertexInputAttributes.
			size());
		VertexInputCreateInfo.pVertexAttributeDescriptions = Description.VertexInputAttributes.data();
		VertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32>(Description.VertexInputBindings.
			size());
		VertexInputCreateInfo.pVertexBindingDescriptions = Description.VertexInputBindings.data();

		VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo = {};
		InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		InputAssemblyCreateInfo.primitiveRestartEnable = false;
		InputAssemblyCreateInfo.topology = Description.Topology;

		VkPipelineViewportStateCreateInfo ViewportCreateInfo = {};
		ViewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ViewportCreateInfo.viewportCount = 1;
		ViewportCreateInfo.pViewports = &Description.Viewport;
		ViewportCreateInfo.scissorCount = 1;
		ViewportCreateInfo.pScissors = &Description.Scissor;

		VkPipelineRasterizationStateCreateInfo RasterizationCreateInfo = {};
		RasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		RasterizationCreateInfo.cullMode = Description.CullMode;
		RasterizationCreateInfo.polygonMode = Description.PolygonMode;
		RasterizationCreateInfo.frontFace = Description.FaceDirection;
		RasterizationCreateInfo.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo MultisampleCreateInfo = {};
		MultisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		MultisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo = {};
		DepthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		DepthStencilCreateInfo.depthTestEnable = Description.IsDepthTestEnabled;
		DepthStencilCreateInfo.depthWriteEnable = Description.IsDepthWriteEnabled;
		DepthStencilCreateInfo.depthCompareOp = Description.DepthCompareOperator;
		DepthStencilCreateInfo.depthBoundsTestEnable = false;
		DepthStencilCreateInfo.stencilTestEnable = false; // TODO : implement

		uint32 ColorAttachmentCount = 0;
		if (RenderPass)
			ColorAttachmentCount = RenderPass->GetColorAttachmentCount();
		else
			ColorAttachmentCount = static_cast<uint32>(ColorAttachmentFormats.size());

		VkPipelineColorBlendStateCreateInfo ColorBlendCreateInfo = {};
		ColorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

		std::vector<VkPipelineColorBlendAttachmentState> AttachmentBlendStates = Description.AttachmentColorBlending;
		// NOTE: if the caller did not specify colo blending states, we need to default to no blending ourselves
		if (AttachmentBlendStates.empty())
		{
			for (uint32 AttachmentIndex = 0; AttachmentIndex < ColorAttachmentCount; AttachmentIndex++)
			{
				VkPipelineColorBlendAttachmentState NewAttachmentBlendState = {};
				NewAttachmentBlendState.blendEnable = false;
				NewAttachmentBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				AttachmentBlendStates.push_back(NewAttachmentBlendState);
			}
		}
		ColorBlendCreateInfo.attachmentCount = static_cast<uint32>(AttachmentBlendStates.size());
		ColorBlendCreateInfo.pAttachments = AttachmentBlendStates.data();
		ColorBlendCreateInfo.blendConstants[0] = Description.BlendingConstants[0];
		ColorBlendCreateInfo.blendConstants[1] = Description.BlendingConstants[1];
		ColorBlendCreateInfo.blendConstants[2] = Description.BlendingConstants[2];
		ColorBlendCreateInfo.blendConstants[3] = Description.BlendingConstants[3];

		VkPipelineDynamicStateCreateInfo DynamicStateInfo = {};
		DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		DynamicStateInfo.dynamicStateCount = static_cast<uint32>(Description.DynamicStates.size());
		DynamicStateInfo.pDynamicStates = Description.DynamicStates.data();
		
		VkGraphicsPipelineCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		CreateInfo.stageCount = static_cast<uint32>(ShaderCreateInfos.size());
		CreateInfo.pStages = ShaderCreateInfos.data();
		CreateInfo.pVertexInputState = &VertexInputCreateInfo;
		CreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;
		CreateInfo.pTessellationState = nullptr;
		CreateInfo.pViewportState = &ViewportCreateInfo;
		CreateInfo.pRasterizationState = &RasterizationCreateInfo;
		CreateInfo.pMultisampleState = &MultisampleCreateInfo;
		CreateInfo.pDepthStencilState = &DepthStencilCreateInfo;
		CreateInfo.pColorBlendState = &ColorBlendCreateInfo;
		CreateInfo.pDynamicState = &DynamicStateInfo;
		CreateInfo.layout = Layout;
		CreateInfo.renderPass = RenderPass ? RenderPass->GetRenderPass() : VK_NULL_HANDLE;
		CreateInfo.subpass = 0;
		CreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		CreateInfo.basePipelineIndex = -1;

		VkPipelineRenderingCreateInfo PipelineRenderingCreateInfo;
		if (DepthAttachmentFormat.has_value() || !ColorAttachmentFormats.empty())
		{
			PipelineRenderingCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.pNext = nullptr,
				.viewMask = 0,
				.colorAttachmentCount = static_cast<uint32>(ColorAttachmentFormats.size()),
				.pColorAttachmentFormats = ColorAttachmentFormats.data(),
				.depthAttachmentFormat = DepthAttachmentFormat.value_or(VK_FORMAT_UNDEFINED),
				.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
			};
			CreateInfo.pNext = &PipelineRenderingCreateInfo;
		}

		vkCreateGraphicsPipelines(Device->Device, VK_NULL_HANDLE, 1, &CreateInfo, GVulkanAllocator, &Handle);
	}
}
