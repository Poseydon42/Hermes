#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Vulkan/Device.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	/*
	 * A safe wrapper around a single Vulkan shader module
	 */
	class HERMES_API Shader
	{
		MAKE_NON_COPYABLE(Shader)
		MAKE_NON_MOVABLE(Shader)

	public:
		Shader(std::shared_ptr<Device::VkDeviceHolder> InDevice, const String& Path, VkShaderStageFlagBits InType);

		~Shader();

		VkShaderModule GetShader() const;

		VkShaderStageFlagBits GetType() const;

	private:
		std::shared_ptr<Device::VkDeviceHolder> Device;

		VkShaderModule Handle = VK_NULL_HANDLE;
		VkShaderStageFlagBits Type;
	};
}
