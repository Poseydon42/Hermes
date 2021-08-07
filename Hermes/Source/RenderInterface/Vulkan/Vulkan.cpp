#define VMA_IMPLEMENTATION
#include "Vulkan.h"

VkAllocationCallbacks* Hermes::Vulkan::GVulkanAllocator = nullptr;
const Hermes::uint32 Hermes::Vulkan::GVulkanVersion = VK_API_VERSION_1_2;
