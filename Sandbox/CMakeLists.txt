cmake_minimum_required(VERSION 3.22)

project(Sandbox)

set(SOURCES
    Source/Sandbox.cpp
)

include(CommonDefinitions)

add_library(Sandbox SHARED ${SOURCES})
add_common_definitions(Sandbox)
target_compile_definitions(Sandbox PRIVATE HERMES_BUILD_APPLICATION)
target_link_libraries(Sandbox PRIVATE Hermes)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
