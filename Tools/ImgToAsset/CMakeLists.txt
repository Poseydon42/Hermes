cmake_minimum_required(VERSION 3.22)

project(ImgToAsset)

set(SOURCES
    Source/Image.cpp
    Source/Image.h
    Source/main.cpp
    Source/PNGLoader.cpp
    Source/PNGLoader.h
)

include(CMakeCommon)

add_executable(ImgToAsset ${SOURCES})
target_include_directories(ImgToAsset PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty ZLIB::ZLIB)
target_compile_definitions(ImgToAsset PRIVATE HERMES_BUILD_TOOLS)
target_link_libraries(ImgToAsset PRIVATE Hermes_Core Hermes_Math ZLIB::ZLIB)

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
