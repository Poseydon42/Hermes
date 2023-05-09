cmake_minimum_required(VERSION 3.24)

set(SOURCES
    DescriptorAllocator.cpp
    DescriptorAllocator.h
    FontPack.cpp
    FontPack.h
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
    Mesh.cpp
    Mesh.h
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
    SceneRenderer.cpp
    SceneRenderer.h
    Scene/Camera.h
    Scene/GeometryList.cpp
    Scene/GeometryList.h
    Scene/Scene.cpp
    Scene/Scene.h
    Scene/SceneNode.cpp
    Scene/SceneNode.h
    ShaderCache.cpp
    ShaderCache.h
    SharedData.h
    Texture.cpp
    Texture.h
    UIRenderer.cpp
    UIRenderer.h
)

include(HermesSublib)

add_engine_sublib(Hermes_RenderingEngine "${SOURCES}")
target_link_libraries(Hermes_RenderingEngine PRIVATE spirv-cross-cpp)
