cmake_minimum_required(VERSION 3.24)

set(HERMES_SUBLIB_NAME Hermes_Math)

set(SOURCES
    BoundingVolume.cpp
    BoundingVolume.h
    Common.h
    Math.h
    Matrix.h
    Plane.h
    Plane.cpp
    Vector.h
    Vector2.h
    Vector4.h
)

include(CMakeCommon)

add_library(${HERMES_SUBLIB_NAME} OBJECT ${SOURCES})

target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_BUILD_ENGINE)
target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_GAME_NAME="Sandbox")

target_include_directories(${HERMES_SUBLIB_NAME} INTERFACE ${HERMES_SOURCE_DIR})
target_link_libraries(${HERMES_SUBLIB_NAME} PRIVATE Hermes_Core)

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
