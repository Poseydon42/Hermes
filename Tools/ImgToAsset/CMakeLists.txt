cmake_minimum_required(VERSION 3.22)

project(ImgToAsset)

set(SOURCES
    Source/main.cpp
)

include(CMakeCommon)

add_executable(ImgToAsset ${SOURCES})
target_include_directories(ImgToAsset PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
