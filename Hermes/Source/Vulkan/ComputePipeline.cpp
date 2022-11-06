#include "ComputePipeline.h"

#include "Vulkan/Descriptor.h"
#include "Vulkan/Shader.h"

namespace Hermes::Vulkan
{
	ComputePipeline::ComputePipeline(std::shared_ptr<Device::VkDeviceHolder> InDevice,
	                                 const std::vector<const DescriptorSetLayout*>& DescriptorSetLayouts,
	                                 const Shader& Shader)
		: Device(std::move(InDevice))
	{
		std::vector<VkDescriptorSetLayout> VkDescriptorSetLayouts(DescriptorSetLayouts.size());
		for (size_t LayoutIndex = 0; LayoutIndex < DescriptorSetLayouts.size(); LayoutIndex++)
		{
			VkDescriptorSetLayouts[LayoutIndex] = DescriptorSetLayouts[LayoutIndex]->GetDescriptorSetLayout();
		}

		VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {};
		PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		PipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32>(VkDescriptorSetLayouts.size());
		PipelineLayoutCreateInfo.pSetLayouts = VkDescriptorSetLayouts.data();
		VK_CHECK_RESULT(vkCreatePipelineLayout(Device->Device, &PipelineLayoutCreateInfo, GVulkanAllocator, &PipelineLayout));

		VkComputePipelineCreateInfo PipelineCreateInfo = {};
		PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

		PipelineCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		PipelineCreateInfo.stage.stage = Shader.GetType();
		PipelineCreateInfo.stage.module = Shader.GetShader();
		PipelineCreateInfo.stage.pName = "main";

		PipelineCreateInfo.layout = PipelineLayout;

		VK_CHECK_RESULT(vkCreateComputePipelines(Device->Device, VK_NULL_HANDLE, 1, &PipelineCreateInfo, GVulkanAllocator, &Pipeline));
	}

	VkPipeline ComputePipeline::GetPipeline() const
	{
		return Pipeline;
	}

	VkPipelineLayout ComputePipeline::GetPipelineLayout() const
	{
		return PipelineLayout;
	}
}
