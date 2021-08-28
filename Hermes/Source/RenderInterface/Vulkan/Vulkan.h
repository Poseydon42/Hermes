#pragma once

#include "Core/Core.h"

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
	extern const uint32 GVulkanVersion;
}

#define VK_CHECK_RESULT(Call) { VkResult VkCheckedResult = Call; if (VkCheckedResult != VK_SUCCESS) { HERMES_LOG_ERROR(L"Vulkan function call failed. Return value is: %u", (unsigned int)VkCheckedResult); } }
