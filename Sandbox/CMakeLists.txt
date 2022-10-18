cmake_minimum_required(VERSION 3.22)

project(Sandbox)

set(SOURCES
    Source/Sandbox.cpp
)

include(CMakeCommon)

add_library(Sandbox SHARED ${SOURCES})
add_compile_definitions(HERMES_BUILD_APPLICATION)
target_link_libraries(Sandbox PRIVATE Hermes)

# TODO: fix this! Hermes target has to expose these properties since Sandbox is linking to it
target_link_libraries(Sandbox PUBLIC $<$<CONFIG:Release>:Tracy::TracyClient>)
target_compile_definitions(Sandbox PRIVATE $<$<CONFIG:Release>:HERMES_ENABLE_PROFILING>)

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
