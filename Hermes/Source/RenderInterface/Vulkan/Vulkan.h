#pragma once

#include "Core/Core.h"
#include "Logging/Logger.h"
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

	inline String VkResultToString(VkResult Result)
	{
		switch (Result)
		{
		case VK_SUCCESS:
			return L"VK_SUCCESS";
		case VK_NOT_READY:
			return L"VK_NOT_READY";
		case VK_TIMEOUT:
			return L"VK_TIMEOUT";
		case VK_EVENT_SET:
			return L"VK_EVENT_SET";
		case VK_EVENT_RESET:
			return L"VK_EVENT_RESET";
		case VK_INCOMPLETE:
			return L"VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return L"VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return L"VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED:
			return L"VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST:
			return L"VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED:
			return L"VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT:
			return L"VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			return L"VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return L"VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return L"VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS:
			return L"VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return L"VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL:
			return L"VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_UNKNOWN:
			return L"VK_ERROR_UNKNOWN";
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			return L"VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			return L"VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case VK_ERROR_FRAGMENTATION:
			return L"VK_ERROR_FRAGMENTATION";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			return L"VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		case VK_ERROR_SURFACE_LOST_KHR:
			return L"VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return L"VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR:
			return L"VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR:
			return L"VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return L"VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT:
			return L"VK_ERROR_VALIDATION_FAILED_EXT";
		case VK_ERROR_INVALID_SHADER_NV:
			return L"VK_ERROR_INVALID_SHADER_NV";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			return L"VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		case VK_ERROR_NOT_PERMITTED_EXT:
			return L"VK_ERROR_NOT_PERMITTED_EXT";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			return L"VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		case VK_THREAD_IDLE_KHR:
			return L"VK_THREAD_IDLE_KHR";
		case VK_THREAD_DONE_KHR:
			return L"VK_THREAD_DONE_KHR";
		case VK_OPERATION_DEFERRED_KHR:
			return L"VK_OPERATION_DEFERRED_KHR";
		case VK_OPERATION_NOT_DEFERRED_KHR:
			return L"VK_OPERATION_NOT_DEFERRED_KHR";
		case VK_PIPELINE_COMPILE_REQUIRED_EXT:
			return L"VK_PIPELINE_COMPILE_REQUIRED_EXT";
		default:
			return L"UNKNOWN_ERROR_CODE";
		}
	}
}

#define VK_CHECK_RESULT(Call) \
	{ \
		VkResult VkCheckedResult = Call; \
		if (VkCheckedResult != VK_SUCCESS) \
		{ \
			HERMES_LOG_ERROR(L"Vulkan function call failed. Error code is %s", VkResultToString(VkCheckedResult).c_str()); \
			::Hermes::PlatformMisc::ExitWithMessageBox((uint32)-1, L"Vulkan error", L"One of Vulkan functions returned VkResult code indicating error. Application could not continue normal execution."); \
		} \
	}
