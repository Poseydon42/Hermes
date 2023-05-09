cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    Component.cpp
    Component.h
    Components/DirectionalLightComponent.h
    Components/MeshComponent.h
    Components/PointLightComponent.h
    Components/TagComponent.h
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

add_engine_sublib(Hermes_World "${SOURCES}")
