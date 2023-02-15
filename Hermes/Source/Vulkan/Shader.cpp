#include "Shader.h"

#include "Platform/GenericPlatform/PlatformFile.h"
#include "VirtualFilesystem/VirtualFilesystem.h"

namespace Hermes::Vulkan
{
	Shader::Shader(std::shared_ptr<Device::VkDeviceHolder> InDevice, const String& Path, VkShaderStageFlagBits InType)
		: Device(std::move(InDevice))
		, Type(InType)
	{
		auto Source = VirtualFilesystem::ReadFileAsBytes(Path);
		if (!Source.has_value())
		{
			HERMES_LOG_ERROR("Failed to open file with shader source. Path: %s", Path.c_str());
			return;
		}

		VkShaderModuleCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		CreateInfo.pCode = reinterpret_cast<const uint32*>(Source.value().data());
		CreateInfo.codeSize = Source.value().size();

		VK_CHECK_RESULT(vkCreateShaderModule(Device->Device, &CreateInfo, GVulkanAllocator, &Handle));
	}

	Shader::~Shader()
	{
		vkDestroyShaderModule(Device->Device, Handle, GVulkanAllocator);
	}

	VkShaderModule Shader::GetShader() const
	{
		return Handle;
	}

	VkShaderStageFlagBits Shader::GetType() const
	{
		return Type;
	}
}
