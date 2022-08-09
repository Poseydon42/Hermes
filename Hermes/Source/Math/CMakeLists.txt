cmake_minimum_required(VERSION 3.24)

set(HERMES_SUBLIB_NAME Hermes_Math)

set(SOURCES
    Common.h
    Math.h
    Matrix.h
    Vector.h
    Vector2.h
    Vector4.h
)

include(CMakeCommon)

add_library(${HERMES_SUBLIB_NAME} INTERFACE ${SOURCES})

target_include_directories(${HERMES_SUBLIB_NAME} INTERFACE ${HERMES_SOURCE_DIR})

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
