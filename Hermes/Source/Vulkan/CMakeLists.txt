cmake_minimum_required(VERSION 3.24)

set(HERMES_SUBLIB_NAME Hermes_Vulkan)

set(SOURCES
    Buffer.cpp
    Buffer.h
    CommandBuffer.cpp
    CommandBuffer.h
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

include(CMakeCommon)

add_library(${HERMES_SUBLIB_NAME} OBJECT ${SOURCES})

target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_BUILD_ENGINE)
target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_GAME_NAME="Sandbox") # TODO : expose as an CMake option later

target_include_directories(${HERMES_SUBLIB_NAME} PUBLIC ${HERMES_SOURCE_DIR})
target_link_libraries(${HERMES_SUBLIB_NAME} PRIVATE Hermes_Core)

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
