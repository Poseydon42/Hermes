#define VMA_IMPLEMENTATION

#include "VulkanCore.h"

namespace Hermes::Vulkan
{
	VkAllocationCallbacks* GVulkanAllocator = nullptr;
	ProfilingMetrics GProfilingMetrics = {};
}

