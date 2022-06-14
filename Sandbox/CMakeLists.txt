cmake_minimum_required(VERSION 3.22)

project(Sandbox)

set(SOURCES
    Source/Sandbox.cpp
)

include(CMakeCommon)

add_library(Sandbox SHARED ${SOURCES})
add_compile_definitions(HERMES_BUILD_APPLICATION)
target_link_libraries(Sandbox PRIVATE Hermes)

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
