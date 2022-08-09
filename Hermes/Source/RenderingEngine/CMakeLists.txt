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
    MeshBuffer.cpp
    MeshBuffer.h
    Passes/GBufferPass.cpp
    Passes/GBufferPass.h
    Passes/PBRPass.cpp
    Passes/PBRPass.h
    Passes/PostProcessingPass.cpp
    Passes/PostProcessingPass.h
    Passes/SkyboxPass.cpp
    Passes/SkyboxPass.h
    Renderer.cpp
    Renderer.h
    Scene/Camera.h
    Scene/FPSCamera.cpp
    Scene/FPSCamera.h
    Scene/Material.cpp
    Scene/Material.h
    Scene/Scene.cpp
    Scene/Scene.h
    Scene/SceneProxies.h
    Texture.cpp
    Texture.h
)

include(CMakeCommon)

add_library(${HERMES_SUBLIB_NAME} OBJECT ${SOURCES})

target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_BUILD_ENGINE)
target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_GAME_NAME="Sandbox") # TODO : expose as an CMake option later

target_include_directories(${HERMES_SUBLIB_NAME} PUBLIC ${HERMES_SOURCE_DIR})

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
