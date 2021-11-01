#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;

		class HERMES_API VulkanShader : public RenderInterface::ShaderBase
		{
			MAKE_NON_COPYABLE(VulkanShader)
		public:
			VulkanShader(std::shared_ptr<const VulkanDevice>InDevice, const String& Path, RenderInterface::ShaderType Type);
			~VulkanShader() override;

			VulkanShader(VulkanShader&& Other);
			VulkanShader& operator=(VulkanShader&& Other);

			VkShaderModule GetShader() const { return Handle; }
			
		private:
			std::shared_ptr<const VulkanDevice> Device;
			VkShaderModule Handle;
		};
	}
}
