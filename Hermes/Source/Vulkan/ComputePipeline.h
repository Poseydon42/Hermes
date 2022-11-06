#pragma once

#include <memory>

#include "Core/Core.h"
#include "Vulkan/Device.h"
#include "Vulkan/Forward.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	class HERMES_API ComputePipeline
	{
	public:
		ComputePipeline(std::shared_ptr<Device::VkDeviceHolder> InDevice,
		                const std::vector<const DescriptorSetLayout*>& DescriptorSetLayouts,
		                const Shader& Shader);

		VkPipeline GetPipeline() const;

		VkPipelineLayout GetPipelineLayout() const;

	private:
		std::shared_ptr<Device::VkDeviceHolder> Device;

		VkPipeline Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
	};
}
