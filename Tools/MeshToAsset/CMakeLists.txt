cmake_minimum_required(VERSION 3.22)

project(MeshToAsset)

set(SOURCES
    Source/main.cpp
)

include(CMakeCommon)

add_executable(MeshToAsset ${SOURCES})

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
