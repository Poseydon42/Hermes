cmake_minimum_required(VERSION 3.22)

project(MeshToAsset)

set(SOURCES
    Source/main.cpp
)

include(CommonDefinitions)

add_executable(MeshToAsset ${SOURCES})
add_common_definitions(MeshToAsset)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
