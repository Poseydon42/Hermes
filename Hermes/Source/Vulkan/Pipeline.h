#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Vulkan/Device.h"
#include "Vulkan/Forward.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	struct PipelineDescription
	{
		/*
		 * Push constants
		 */
		std::vector<VkPushConstantRange> PushConstants;

		/*
		 * Descriptor set layouts
		 */
		std::vector<const DescriptorSetLayout*> DescriptorSetLayouts;

		/*
		 * Shader stages
		 */
		std::vector<const Shader*> ShaderStages;

		/*
		 * Vertex input
		 */
		std::vector<VkVertexInputBindingDescription> VertexInputBindings;
		std::vector<VkVertexInputAttributeDescription> VertexInputAttributes;

		/*
		 * Input assembly
		 */
		VkPrimitiveTopology Topology;

		/*
		 * Viewport
		 */
		VkViewport Viewport;
		VkRect2D Scissor;

		/*
		 * Rasterization
		 */
		VkPolygonMode PolygonMode;
		VkCullModeFlags CullMode;
		VkFrontFace FaceDirection;

		/*
		 * Blending
		 *
		 * NOTE: if this vector is uninitialized then pipeline will be constructed with color blending disabled
		 */
		std::vector<VkPipelineColorBlendAttachmentState> AttachmentColorBlending;
		float BlendingConstants[4];

		/*
		 * Depth-stencil
		 */
		bool IsDepthTestEnabled;
		bool IsDepthWriteEnabled;
		VkCompareOp DepthCompareOperator;

		/*
		 * Dynamic states (things that can be changed before every draw call)
		 */
		std::vector<VkDynamicState> DynamicStates;
	};

	class HERMES_API Pipeline
	{
		MAKE_NON_COPYABLE(Pipeline)
		MAKE_NON_MOVABLE(Pipeline)

	public:
		Pipeline(std::shared_ptr<Device::VkDeviceHolder> InDevice, const RenderPass& RenderPass, const PipelineDescription& Description);

		Pipeline(std::shared_ptr<Device::VkDeviceHolder> InDevice, const PipelineDescription& Description, std::span<const VkFormat> ColorAttachmentFormats, std::optional<VkFormat> DepthAttachmentFormat);

		~Pipeline();

		VkPipeline GetPipeline() const;

		VkPipelineLayout GetPipelineLayout() const;

	private:
		Pipeline(std::shared_ptr<Device::VkDeviceHolder> InDevice, const PipelineDescription& Description, const RenderPass* RenderPass, std::span<const VkFormat> ColorAttachmentFormats, std::optional<VkFormat> DepthAttachmentFormat);

		std::shared_ptr<Device::VkDeviceHolder> Device;

		VkPipeline Handle = VK_NULL_HANDLE;
		VkPipelineLayout Layout = VK_NULL_HANDLE;
	};
}
