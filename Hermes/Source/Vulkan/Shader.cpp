#include "Shader.h"

#include "Platform/GenericPlatform/PlatformFile.h"

namespace Hermes::Vulkan
{
	Shader::Shader(std::shared_ptr<Device::VkDeviceHolder> InDevice, const String& Path, VkShaderStageFlagBits InType)
		: Device(std::move(InDevice))
		, Type(InType)
	{
		std::shared_ptr<IPlatformFile> SourceFile =
			PlatformFilesystem::OpenFile(Path, IPlatformFile::FileAccessMode::Read,
			                             IPlatformFile::FileOpenMode::OpenExisting);
		if (!SourceFile)
		{
			HERMES_LOG_ERROR("Failed to open file with shader source. Path: %s", Path.c_str());
			return;
		}
		size_t FileSize = SourceFile->Size();
		auto* Source = static_cast<uint8*>(malloc(FileSize));
		if (!Source)
		{
			HERMES_LOG_ERROR("Failed to allocate memory for shader source. Path: %s", Path.c_str());
			return;
		}
		if (!SourceFile->Read(Source, FileSize))
		{
			HERMES_LOG_ERROR("Failed to read shader source file. Path: %s", Path.c_str());
			return;
		}

		VkShaderModuleCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		CreateInfo.pCode = reinterpret_cast<const uint32*>(Source);
		CreateInfo.codeSize = FileSize;

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
