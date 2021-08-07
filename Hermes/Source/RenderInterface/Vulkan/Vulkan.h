#pragma once

#ifdef HERMES_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Core/Core.h"

namespace Hermes::Vulkan
{
	extern VkAllocationCallbacks* GVulkanAllocator;
	extern const uint32 GVulkanVersion;
}

#define VK_CHECK_RESULT(Call) { VkResult VkCheckedResult = Call; if (VkCheckedResult != VK_SUCCESS) { HERMES_LOG_ERROR(L"Vulkan function call failed. Return value is: %u", (unsigned int)VkCheckedResult); } }
