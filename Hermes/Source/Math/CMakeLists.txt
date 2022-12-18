cmake_minimum_required(VERSION 3.24)

set(SOURCES
    BoundingVolume.cpp
    BoundingVolume.h
    Common.h
    Frustum.cpp
    Frustum.h
    Math.h
    Matrix.h
    Plane.h
    Plane.cpp
    Vector.h
    Vector2.h
    Vector4.h
)

include(HermesSublib)

add_hermes_sublib(Hermes_Math "${SOURCES}")
target_link_libraries(Hermes_Math PUBLIC Hermes_Core)
