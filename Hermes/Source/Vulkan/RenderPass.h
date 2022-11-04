#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Vulkan/Device.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	enum class AttachmentType
	{
		Color,
		Input,
		DepthStencil
	};

	/*
	 * A safe wrapper around VkRenderPass
	 *
	 * At the moment supports render passes with a single subpass only
	 */
	class RenderPass
	{
		MAKE_NON_COPYABLE(RenderPass)
		MAKE_NON_MOVABLE(RenderPass)

	public:
		RenderPass(std::shared_ptr<Device::VkDeviceHolder> InDevice,
		           const std::vector<std::pair<VkAttachmentDescription, AttachmentType>>& Attachments);
			
		~RenderPass();

		uint32 GetColorAttachmentCount() const;
			
		VkRenderPass GetRenderPass() const;
		
	private:
		std::shared_ptr<Device::VkDeviceHolder> Device;

		VkRenderPass Handle = VK_NULL_HANDLE;
		uint32 ColorAttachmentCount = 0;
	};
}
