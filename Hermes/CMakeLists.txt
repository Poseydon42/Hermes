cmake_minimum_required(VERSION 3.22)

project(Hermes)

set(SOURCES
    Source/ApplicationCore/Application.h
    Source/ApplicationCore/Event.h
    Source/ApplicationCore/EventQueue.cpp
    Source/ApplicationCore/EventQueue.h
    Source/ApplicationCore/GameLoop.cpp
    Source/ApplicationCore/GameLoop.h
    Source/ApplicationCore/InputEngine.cpp
    Source/ApplicationCore/InputEngine.h
    Source/AssetSystem/Asset.cpp
    Source/AssetSystem/Asset.h
    Source/AssetSystem/AssetLoader.cpp
    Source/AssetSystem/AssetLoader.h
    Source/AssetSystem/ImageAsset.cpp
    Source/AssetSystem/ImageAsset.h
    Source/AssetSystem/MeshAsset.cpp
    Source/AssetSystem/MeshAsset.h
    Source/Core/Compiler/CompilerMSVC.h
    Source/Core/Core.h
    Source/Core/Delegate/Delegate.h
    Source/Core/Log/DebugLogDevice.h
    Source/Core/Log/FileLogDevice.cpp
    Source/Core/Log/FileLogDevice.h
    Source/Core/Log/ILogDevice.h
    Source/Core/Log/Logger.cpp
    Source/Core/Log/Logger.h
    Source/Core/Misc/DefaultConstructors.h
    Source/Core/Misc/EnumClassOperators.h
    Source/Core/Misc/NonCopyableMovable.h
    Source/Core/Misc/StringUtils.cpp
    Source/Core/Misc/StringUtils.h
    Source/Launch/LaunchWindows.cpp
    Source/Math/Common.h
    Source/Math/Math.h
    Source/Math/Matrix.h
    Source/Math/Vector.h
    Source/Math/Vector2.h
    Source/Math/Vector4.h
    Source/Platform/GenericPlatform/PlatformDebug.h
    Source/Platform/GenericPlatform/PlatformFile.h
    Source/Platform/GenericPlatform/PlatformLibrary.h
    Source/Platform/GenericPlatform/PlatformMisc.h
    Source/Platform/GenericPlatform/PlatformTime.h
    Source/Platform/GenericPlatform/PlatformWindow.h
    Source/Platform/Windows/WindowsDebug.cpp
    Source/Platform/Windows/WindowsFile.cpp
    Source/Platform/Windows/WindowsFile.h
    Source/Platform/Windows/WindowsLibrary.cpp
    Source/Platform/Windows/WindowsLibrary.h
    Source/Platform/Windows/WindowsMisc.cpp
    Source/Platform/Windows/WindowsTime.cpp
    Source/Platform/Windows/WindowsTime.h
    Source/Platform/Windows/WindowsWindow.cpp
    Source/Platform/Windows/WindowsWindow.h
    Source/RenderingEngine/DescriptorAllocator.cpp
    Source/RenderingEngine/DescriptorAllocator.h
    Source/RenderingEngine/FrameGraph/Graph.cpp
    Source/RenderingEngine/FrameGraph/Graph.h
    Source/RenderingEngine/FrameGraph/Pass.h
    Source/RenderingEngine/FrameGraph/Resource.cpp
    Source/RenderingEngine/FrameGraph/Resource.h
    Source/RenderingEngine/GPUInteractionUtilities.cpp
    Source/RenderingEngine/GPUInteractionUtilities.h
    Source/RenderingEngine/MeshBuffer.cpp
    Source/RenderingEngine/MeshBuffer.h
    Source/RenderingEngine/Passes/GBufferPass.cpp
    Source/RenderingEngine/Passes/GBufferPass.h
    Source/RenderingEngine/Passes/PBRPass.cpp
    Source/RenderingEngine/Passes/PBRPass.h
    Source/RenderingEngine/Passes/PostProcessingPass.cpp
    Source/RenderingEngine/Passes/PostProcessingPass.h
    Source/RenderingEngine/Passes/SkyboxPass.cpp
    Source/RenderingEngine/Passes/SkyboxPass.h
    Source/RenderingEngine/Renderer.cpp
    Source/RenderingEngine/Renderer.h
    Source/RenderingEngine/Scene/Camera.h
    Source/RenderingEngine/Scene/FPSCamera.cpp
    Source/RenderingEngine/Scene/FPSCamera.h
    Source/RenderingEngine/Scene/Material.cpp
    Source/RenderingEngine/Scene/Material.h
    Source/RenderingEngine/Scene/Scene.cpp
    Source/RenderingEngine/Scene/Scene.h
    Source/RenderingEngine/Scene/SceneProxies.h
    Source/RenderingEngine/Texture.cpp
    Source/RenderingEngine/Texture.h
    Source/RenderInterface/GenericRenderInterface/Buffer.h
    Source/RenderInterface/GenericRenderInterface/CommandBuffer.h
    Source/RenderInterface/GenericRenderInterface/CommonTypes.h
    Source/RenderInterface/GenericRenderInterface/Descriptor.h
    Source/RenderInterface/GenericRenderInterface/Device.h
    Source/RenderInterface/GenericRenderInterface/Fence.h
    Source/RenderInterface/GenericRenderInterface/Forward.h
    Source/RenderInterface/GenericRenderInterface/Image.h
    Source/RenderInterface/GenericRenderInterface/Instance.cpp
    Source/RenderInterface/GenericRenderInterface/Instance.h
    Source/RenderInterface/GenericRenderInterface/PhysicalDevice.h
    Source/RenderInterface/GenericRenderInterface/Pipeline.h
    Source/RenderInterface/GenericRenderInterface/Queue.h
    Source/RenderInterface/GenericRenderInterface/RenderPass.h
    Source/RenderInterface/GenericRenderInterface/RenderTarget.h
    Source/RenderInterface/GenericRenderInterface/Sampler.h
    Source/RenderInterface/GenericRenderInterface/Shader.h
    Source/RenderInterface/GenericRenderInterface/Swapchain.h
    Source/RenderInterface/Vulkan/Vulkan.cpp
    Source/RenderInterface/Vulkan/Vulkan.h
    Source/RenderInterface/Vulkan/VulkanBuffer.cpp
    Source/RenderInterface/Vulkan/VulkanBuffer.h
    Source/RenderInterface/Vulkan/VulkanCommandBuffer.cpp
    Source/RenderInterface/Vulkan/VulkanCommandBuffer.h
    Source/RenderInterface/Vulkan/VulkanCommonTypes.h
    Source/RenderInterface/Vulkan/VulkanDescriptor.cpp
    Source/RenderInterface/Vulkan/VulkanDescriptor.h
    Source/RenderInterface/Vulkan/VulkanDevice.cpp
    Source/RenderInterface/Vulkan/VulkanDevice.h
    Source/RenderInterface/Vulkan/VulkanFence.cpp
    Source/RenderInterface/Vulkan/VulkanFence.h
    Source/RenderInterface/Vulkan/VulkanImage.cpp
    Source/RenderInterface/Vulkan/VulkanImage.h
    Source/RenderInterface/Vulkan/VulkanInstance.cpp
    Source/RenderInterface/Vulkan/VulkanInstance.h
    Source/RenderInterface/Vulkan/VulkanPhysicalDevice.cpp
    Source/RenderInterface/Vulkan/VulkanPhysicalDevice.h
    Source/RenderInterface/Vulkan/VulkanPipeline.cpp
    Source/RenderInterface/Vulkan/VulkanPipeline.h
    Source/RenderInterface/Vulkan/VulkanQueue.cpp
    Source/RenderInterface/Vulkan/VulkanQueue.h
    Source/RenderInterface/Vulkan/VulkanRenderPass.cpp
    Source/RenderInterface/Vulkan/VulkanRenderPass.h
    Source/RenderInterface/Vulkan/VulkanRenderTarget.cpp
    Source/RenderInterface/Vulkan/VulkanRenderTarget.h
    Source/RenderInterface/Vulkan/VulkanSampler.cpp
    Source/RenderInterface/Vulkan/VulkanSampler.h
    Source/RenderInterface/Vulkan/VulkanShader.cpp
    Source/RenderInterface/Vulkan/VulkanShader.h
    Source/RenderInterface/Vulkan/VulkanSwapchain.cpp
    Source/RenderInterface/Vulkan/VulkanSwapchain.h
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
