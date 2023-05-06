cmake_minimum_required(VERSION 3.24)

set(SOURCES
    BoundingVolume.cpp
    BoundingVolume.h
    Common.h
    FromString.cpp
    FromString.h
    Frustum.cpp
    Frustum.h
    Math.h
    Matrix.h
    Plane.h
    Plane.cpp
    Quaternion.cpp
    Quaternion.h
    Rect2D.h
    Transform.cpp
    Transform.h
    Vector.h
    Vector2.h
    Vector4.h
)

include(HermesSublib)

add_hermes_sublib(Hermes_Math "${SOURCES}")
