cmake_minimum_required(VERSION 3.24)

set(HERMES_SUBLIB_NAME Hermes_RenderInterface)

set(SOURCES
    GenericRenderInterface/Buffer.h
    GenericRenderInterface/CommandBuffer.h
    GenericRenderInterface/CommonTypes.h
    GenericRenderInterface/Descriptor.h
    GenericRenderInterface/Device.h
    GenericRenderInterface/Fence.h
    GenericRenderInterface/Forward.h
    GenericRenderInterface/Image.h
    GenericRenderInterface/Instance.cpp
    GenericRenderInterface/Instance.h
    GenericRenderInterface/PhysicalDevice.h
    GenericRenderInterface/Pipeline.h
    GenericRenderInterface/Queue.h
    GenericRenderInterface/RenderPass.h
    GenericRenderInterface/RenderTarget.h
    GenericRenderInterface/Sampler.h
    GenericRenderInterface/Shader.h
    GenericRenderInterface/Swapchain.h
    Vulkan/Vulkan.cpp
    Vulkan/Vulkan.h
    Vulkan/VulkanBuffer.cpp
    Vulkan/VulkanBuffer.h
    Vulkan/VulkanCommandBuffer.cpp
    Vulkan/VulkanCommandBuffer.h
    Vulkan/VulkanCommonTypes.h
    Vulkan/VulkanDescriptor.cpp
    Vulkan/VulkanDescriptor.h
    Vulkan/VulkanDevice.cpp
    Vulkan/VulkanDevice.h
    Vulkan/VulkanFence.cpp
    Vulkan/VulkanFence.h
    Vulkan/VulkanImage.cpp
    Vulkan/VulkanImage.h
    Vulkan/VulkanInstance.cpp
    Vulkan/VulkanInstance.h
    Vulkan/VulkanPhysicalDevice.cpp
    Vulkan/VulkanPhysicalDevice.h
    Vulkan/VulkanPipeline.cpp
    Vulkan/VulkanPipeline.h
    Vulkan/VulkanQueue.cpp
    Vulkan/VulkanQueue.h
    Vulkan/VulkanRenderPass.cpp
    Vulkan/VulkanRenderPass.h
    Vulkan/VulkanRenderTarget.cpp
    Vulkan/VulkanRenderTarget.h
    Vulkan/VulkanSampler.cpp
    Vulkan/VulkanSampler.h
    Vulkan/VulkanShader.cpp
    Vulkan/VulkanShader.h
    Vulkan/VulkanSwapchain.cpp
    Vulkan/VulkanSwapchain.h
)

include(CMakeCommon)

find_package(Vulkan REQUIRED)

add_library(${HERMES_SUBLIB_NAME} OBJECT ${SOURCES})

target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_BUILD_ENGINE)
target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_GAME_NAME="Sandbox") # TODO : expose as an CMake option later
target_include_directories(${HERMES_SUBLIB_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_include_directories(${HERMES_SUBLIB_NAME} PRIVATE "${HERMES_THIRD_PARTY_DIR}/VulkanMemoryAllocator/include")
target_link_libraries(${HERMES_SUBLIB_NAME} PRIVATE Vulkan::Vulkan)

target_include_directories(${HERMES_SUBLIB_NAME} PUBLIC ${HERMES_SOURCE_DIR})

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
