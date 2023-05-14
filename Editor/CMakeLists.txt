cmake_minimum_required(VERSION 3.24)

project(Editor C CXX)

set(HERMES_EDITOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")

add_subdirectory(Source/Core)

set(EDITOR_SUBLIB_LIST
    Editor_Core
)

add_library(Editor SHARED Source/EditorExecutable.cpp)
target_link_libraries(Editor PRIVATE ${EDITOR_SUBLIB_LIST})
target_link_libraries(Editor PRIVATE Hermes)

# Hacks to prevent MSVC linker from omitting "unused" symbols
if (MSVC)
    target_link_options(Editor PRIVATE "/OPT:NOREF")
    foreach(SUBLIB IN LISTS EDITOR_SUBLIB_LIST)
        target_link_options(Editor PRIVATE "/WHOLEARCHIVE:${SUBLIB}.lib")
    endforeach(SUBLIB in ${EDITOR_SUBLIB_LIST})
endif()
