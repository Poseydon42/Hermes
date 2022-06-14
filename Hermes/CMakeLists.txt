cmake_minimum_required(VERSION 3.22)

project(Hermes)

set(SOURCES
    Source/AssetSystem/Asset.cpp
    Source/AssetSystem/AssetLoader.cpp
    Source/AssetSystem/ImageAsset.cpp
    Source/AssetSystem/MeshAsset.cpp
    Source/Core/Application/EventQueue.cpp
    Source/Core/Application/GameLoop.cpp
    Source/Core/Application/InputEngine.cpp
    Source/Core/Launch/LaunchWindows.cpp
    Source/Core/Log/FileLogDevice.cpp
    Source/Core/Log/Logger.cpp
    Source/Core/Misc/StringUtils.cpp
    Source/Platform/Windows/WindowsDebug.cpp
    Source/Platform/Windows/WindowsFile.cpp
    Source/Platform/Windows/WindowsLibrary.cpp
    Source/Platform/Windows/WindowsMisc.cpp
    Source/Platform/Windows/WindowsTime.cpp
    Source/Platform/Windows/WindowsWindow.cpp
    Source/RenderingEngine/DescriptorAllocator.cpp
    Source/RenderingEngine/FrameGraph/Graph.cpp
    Source/RenderingEngine/FrameGraph/Resource.cpp
    Source/RenderingEngine/GPUInteractionUtilities.cpp
    Source/RenderingEngine/MeshBuffer.cpp
    Source/RenderingEngine/Passes/GBufferPass.cpp
    Source/RenderingEngine/Passes/SkyboxPass.cpp
    Source/RenderingEngine/Renderer.cpp
    Source/RenderingEngine/Scene/FPSCamera.cpp
    Source/RenderingEngine/Scene/Material.cpp
    Source/RenderingEngine/Scene/Scene.cpp
    Source/RenderingEngine/Texture.cpp
    Source/RenderInterface/GenericRenderInterface/Instance.cpp
    Source/RenderInterface/Vulkan/Vulkan.cpp
    Source/RenderInterface/Vulkan/VulkanBuffer.cpp
    Source/RenderInterface/Vulkan/VulkanCommandBuffer.cpp
    Source/RenderInterface/Vulkan/VulkanDescriptor.cpp
    Source/RenderInterface/Vulkan/VulkanDevice.cpp
    Source/RenderInterface/Vulkan/VulkanFence.cpp
    Source/RenderInterface/Vulkan/VulkanImage.cpp
    Source/RenderInterface/Vulkan/VulkanInstance.cpp
    Source/RenderInterface/Vulkan/VulkanPhysicalDevice.cpp
    Source/RenderInterface/Vulkan/VulkanPipeline.cpp
    Source/RenderInterface/Vulkan/VulkanQueue.cpp
    Source/RenderInterface/Vulkan/VulkanRenderPass.cpp
    Source/RenderInterface/Vulkan/VulkanRenderTarget.cpp
    Source/RenderInterface/Vulkan/VulkanSampler.cpp
    Source/RenderInterface/Vulkan/VulkanShader.cpp
    Source/RenderInterface/Vulkan/VulkanSwapchain.cpp
)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIR})

include(CMakeCommon)

add_executable(Hermes WIN32 ${SOURCES})

set_target_properties(Hermes PROPERTIES ENABLE_EXPORTS TRUE)
target_link_libraries(Hermes PRIVATE Vulkan::Vulkan)
target_include_directories(Hermes PRIVATE "ThirdParty/VulkanMemoryAllocator/include")
add_compile_definitions(HERMES_BUILD_ENGINE)
add_compile_definitions(HERMES_GAME_NAME="Sandbox") # TODO : expose as an CMake option later

target_include_directories(Hermes PUBLIC "Source")

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
