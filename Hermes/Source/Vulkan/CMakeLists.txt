cmake_minimum_required(VERSION 3.24)

set(SOURCES
    Buffer.cpp
    Buffer.h
    CommandBuffer.cpp
    CommandBuffer.h
    ComputePipeline.cpp
    ComputePipeline.h
    Descriptor.cpp
    Descriptor.h
    Device.cpp
    Device.h
    Fence.cpp
    Fence.h
    Forward.h
    Framebuffer.cpp
    Framebuffer.h
    Image.cpp
    Image.h
    Instance.cpp
    Instance.h
    Pipeline.cpp
    Pipeline.h
    Queue.cpp
    Queue.h
    RenderPass.cpp
    RenderPass.h
    Sampler.cpp
    Sampler.h
    Shader.cpp
    Shader.h
    Swapchain.cpp
    Swapchain.h
    VulkanCore.cpp
    VulkanCore.h
)

include(HermesSublib)

add_hermes_sublib(Hermes_Vulkan "${SOURCES}")

target_include_directories(Hermes_Vulkan PUBLIC "${HERMES_THIRD_PARTY_DIR}/VulkanMemoryAllocator/include")
target_link_libraries(Hermes_Vulkan PUBLIC Vulkan::Vulkan)
