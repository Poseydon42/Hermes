#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Math/Vector2.h"
#include "Vulkan/Device.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	/*
	 * A wrapper around VkFramebuffer object.
	 *
	 * Does not hold an ownership of its attachments, so the user needs to take care of it themselves
	 */
	class HERMES_API Framebuffer
	{
		MAKE_NON_COPYABLE(Framebuffer)
		MAKE_NON_MOVABLE(Framebuffer)

	public:
		Framebuffer(std::shared_ptr<Device::VkDeviceHolder> InDevice, const RenderPass& InRenderPass,
		             const std::vector<const ImageView*>& InAttachments, Vec2ui InDimensions);

		~Framebuffer();

		Vec2ui GetDimensions() const;
			
		uint32 GetImageCount() const;

		VkFramebuffer GetFramebuffer() const;

	private:
		std::shared_ptr<Device::VkDeviceHolder> Device;

		VkFramebuffer Handle = VK_NULL_HANDLE;
		uint32 ImageCount = 0;
		Vec2ui Dimensions;
	};
}
