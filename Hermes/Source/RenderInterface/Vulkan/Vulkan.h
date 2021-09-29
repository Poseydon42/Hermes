#pragma once

#include "Core/Core.h"
#include "Platform/GenericPlatform/PlatformMisc.h"

#ifdef HERMES_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#endif
// !!!!! DON'T EXCHANGE THOSE TWO INCLUDES. vk_mem_alloc.h causes lot's of build error if it is included after Vulkan headers !!!!!
SUPPRESS_ALL_WARNINGS_BEGIN
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
SUPPRESS_ALL_WARNINGS_END

namespace Hermes::Vulkan
{
	extern VkAllocationCallbacks* GVulkanAllocator;
	constexpr uint32 GVulkanVersion = VK_API_VERSION_1_1;
}

#define VK_CHECK_RESULT(Call) \
	{ \
		VkResult VkCheckedResult = Call; \
		if (VkCheckedResult != VK_SUCCESS) \
		{ \
			HERMES_LOG_ERROR(L"Vulkan function call failed. Return value is: %u", (unsigned int)VkCheckedResult); \
			::Hermes::PlatformMisc::ExitWithMessageBox((uint32)-1, L"Vulkan error", L"One of Vulkan functions returned VkResult code indicating error. Application could not continue normal execution."); \
		} \
	}
