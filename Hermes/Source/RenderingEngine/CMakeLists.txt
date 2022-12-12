cmake_minimum_required(VERSION 3.24)

set(HERMES_SUBLIB_NAME Hermes_RenderingEngine)

set(SOURCES
    DescriptorAllocator.cpp
    DescriptorAllocator.h
    FrameGraph/Graph.cpp
    FrameGraph/Graph.h
    FrameGraph/Pass.h
    FrameGraph/Resource.cpp
    FrameGraph/Resource.h
    GPUInteractionUtilities.cpp
    GPUInteractionUtilities.h
    Material/Material.cpp
    Material/Material.h
    Material/MaterialInstance.cpp
    Material/MaterialInstance.h
    Material/MaterialProperty.cpp
    Material/MaterialProperty.h
    Material/ShaderReflection.cpp
    Material/ShaderReflection.h
    MeshBuffer.cpp
    MeshBuffer.h
    Passes/DepthPass.cpp
    Passes/DepthPass.h
    Passes/ForwardPass.cpp
    Passes/ForwardPass.h
    Passes/LightCullingPass.cpp
    Passes/LightCullingPass.h
    Passes/PostProcessingPass.cpp
    Passes/PostProcessingPass.h
    Passes/SkyboxPass.cpp
    Passes/SkyboxPass.h
    Renderer.cpp
    Renderer.h
    Scene/Camera.h
    Scene/FPSCamera.cpp
    Scene/FPSCamera.h
    Scene/GeometryList.cpp
    Scene/GeometryList.h
    Scene/Scene.cpp
    Scene/Scene.h
    Scene/SceneProxies.h
    SharedData.h
    Texture.cpp
    Texture.h
)

include(CMakeCommon)

add_library(${HERMES_SUBLIB_NAME} OBJECT ${SOURCES})

target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_BUILD_ENGINE)
target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_GAME_NAME="Sandbox") # TODO : expose as an CMake option later

target_include_directories(${HERMES_SUBLIB_NAME} PUBLIC ${HERMES_SOURCE_DIR})
target_link_libraries(${HERMES_SUBLIB_NAME} PRIVATE spirv-cross-cpp Hermes_Core Hermes_Vulkan)

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
