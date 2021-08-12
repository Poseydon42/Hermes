#include "VulkanShader.h"

#include "VulkanDevice.h"
#include "Platform/GenericPlatform/PlatformFile.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanShader::VulkanShader(std::shared_ptr<VulkanDevice> InDevice, const String& Path, RenderInterface::ShaderType Type)
			: ShaderBase(Type)
			, Device(std::move(InDevice))
			, Handle(VK_NULL_HANDLE)
		{
			std::shared_ptr<IPlatformFile> SourceFile = PlatformFilesystem::OpenFile(Path, IPlatformFile::FileAccessMode::Read, IPlatformFile::FileOpenMode::OpenExisting);
			if (!SourceFile)
			{
				HERMES_LOG_ERROR(L"Failed to open file with shader source. Path: %s", Path.c_str());
				return;
			}
			size_t FileSize = SourceFile->Size();
			auto* Source = (uint8*)malloc(FileSize);
			if (!Source)
			{
				HERMES_LOG_ERROR(L"Failed to allocate memory for shader source. Path: %s", Path.c_str());
				return;
			}
			if (!SourceFile->Read(Source, FileSize))
			{
				HERMES_LOG_ERROR(L"Failed to read shader source file. Path: %s", Path.c_str());
				return;
			}

			VkShaderModuleCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			CreateInfo.pCode = (const uint32*)Source;
			CreateInfo.codeSize = FileSize;

			VK_CHECK_RESULT(vkCreateShaderModule(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &Handle));
		}

		VulkanShader::~VulkanShader()
		{
			vkDestroyShaderModule(Device->GetDevice(), Handle, GVulkanAllocator);
		}

		VulkanShader::VulkanShader(VulkanShader&& Other)
			: ShaderBase(Other.GetType())
		{
			*this = std::move(Other);
		}

		VulkanShader& VulkanShader::operator=(VulkanShader&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Handle, Other.Handle);
			return *this;
		}
	}
}
