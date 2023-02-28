cmake_minimum_required(VERSION 3.22)

project(MeshToAsset)

set(SOURCES
    Source/FileProcessor.cpp
    Source/FileProcessor.h
    Source/GLTFReader.cpp
    Source/GLTFReader.h
    Source/InputFileReader.h
    Source/main.cpp
    Source/Mesh.cpp
    Source/Mesh.h
    Source/MeshWriter.cpp
    Source/MeshWriter.h
    Source/Node.h
    Source/Node.cpp
    Source/OBJReader.cpp
    Source/OBJReader.h
)

include(CommonDefinitions)

add_executable(MeshToAsset ${SOURCES})
target_include_directories(MeshToAsset PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
target_compile_definitions(MeshToAsset PRIVATE HERMES_BUILD_TOOLS)
target_link_libraries(MeshToAsset PRIVATE Hermes_AssetSystem Hermes_Core Hermes_Logging Hermes_Math Hermes_Platform)
add_common_definitions(MeshToAsset)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
