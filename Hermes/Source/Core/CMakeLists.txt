cmake_minimum_required(VERSION 3.24)

set(HERMES_SUBLIB_NAME Hermes_Core)

set(SOURCES
    Compiler/CompilerMSVC.h
    Core.h
    Delegate/Delegate.h
    Misc/DefaultConstructors.h
    Misc/EnumClassOperators.h
    Misc/NonCopyableMovable.h
    Misc/ArgsParser.cpp
    Misc/ArgsParser.h
    Misc/StringUtils.cpp
    Misc/StringUtils.h
)

include(CMakeCommon)

add_library(${HERMES_SUBLIB_NAME} OBJECT ${SOURCES})

target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_BUILD_ENGINE)
target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_GAME_NAME="Sandbox") # TODO : expose as an CMake option later

target_include_directories(${HERMES_SUBLIB_NAME} PUBLIC ${HERMES_SOURCE_DIR})

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
