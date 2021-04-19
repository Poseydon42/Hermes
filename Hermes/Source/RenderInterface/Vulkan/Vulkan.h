#pragma once

#ifdef HERMES_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

namespace Hermes::Vulkan
{
	extern VkAllocationCallbacks* GVulkanAllocator;
}

#define VK_CHECK_RESULT(Call) { VkResult VkCheckedResult = Call; if (VkCheckedResult != VK_SUCCESS) { HERMES_LOG_ERROR(L"Vulkan function call failed. Return value is: %u", (unsigned int)VkCheckedResult); } }
