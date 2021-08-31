#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Vulkan.h"
#include "Math/Vector2.h"
#include "RenderInterface/GenericRenderInterface/Image.h"
#include "RenderInterface/GenericRenderInterface/RenderTarget.h"
#include "RenderInterface/GenericRenderInterface/RenderPass.h"


namespace Hermes
{
	namespace Vulkan
	{
		class VulkanImage;
		class VulkanRenderPass;
		class VulkanDevice;

		class HERMES_API VulkanRenderTarget : public RenderInterface::RenderTarget
		{
			MAKE_NON_COPYABLE(VulkanRenderTarget)

		public:
			VulkanRenderTarget(std::shared_ptr<VulkanDevice> InDevice, const std::shared_ptr<RenderInterface::RenderPass>& InRenderPass, std::vector<std::shared_ptr<RenderInterface::Image>> InAttachments, Vec2ui InSize);

			~VulkanRenderTarget() override;
			VulkanRenderTarget(VulkanRenderTarget&& Other);
			VulkanRenderTarget& operator=(VulkanRenderTarget&& Other);

			Vec2ui GetSize() const override;
			
			uint32 GetImageCount() const override;

			VkFramebuffer GetFramebuffer() const;

		private:
			std::shared_ptr<VulkanDevice> Device;
			std::shared_ptr<VulkanRenderPass> RenderPass;
			std::vector<std::shared_ptr<VulkanImage>> Attachments;
			VkFramebuffer Framebuffer;
			Vec2ui Size;
		};
	}
}
