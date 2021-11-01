#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"
#include "RenderInterface/GenericRenderInterface/RenderPass.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanRenderPass;
		class VulkanDevice;

		class HERMES_API VulkanPipeline : public RenderInterface::Pipeline
		{
			MAKE_NON_COPYABLE(VulkanPipeline)

		public:
			VulkanPipeline(std::shared_ptr<const VulkanDevice> InDevice, std::shared_ptr<RenderInterface::RenderPass> InRenderPass, const RenderInterface::PipelineDescription& Description);
			
			~VulkanPipeline();
			VulkanPipeline(VulkanPipeline&& Other);
			VulkanPipeline& operator=(VulkanPipeline&& Other);

			VkPipeline GetPipeline() const;

			VkPipelineLayout GetPipelineLayout() const;

		private:
			std::shared_ptr<const VulkanDevice> Device;
			std::shared_ptr<VulkanRenderPass> RenderPass;
			VkPipeline Pipeline;
			VkPipelineLayout Layout;
		};
	}
}
