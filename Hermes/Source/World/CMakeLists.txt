cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    Component.h
    Components/DirectionalLightComponent.h
    Components/MeshComponent.h
    Components/PointLightComponent.h
    Components/TransformComponent.h
    Entity.h
    Systems/LightRenderingSystem.cpp
    Systems/LightRenderingSystem.h
    Systems/MeshRenderingSystem.cpp
    Systems/MeshRenderingSystem.h
    System.h
    World.cpp
    World.h
)

add_hermes_sublib(Hermes_World "${SOURCES}")
target_link_libraries(Hermes_World PUBLIC Hermes_Core Hermes_RenderingEngine)
