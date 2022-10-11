cmake_minimum_required(VERSION 3.24)

set(HERMES_SUBLIB_NAME Hermes_Platform)

set(SOURCES
    GenericPlatform/PlatformDebug.h
    GenericPlatform/PlatformFile.h
    GenericPlatform/PlatformLibrary.h
    GenericPlatform/PlatformMisc.h
    GenericPlatform/PlatformTime.h
    GenericPlatform/PlatformWindow.h
    Windows/WindowsDebug.cpp
    Windows/WindowsFile.cpp
    Windows/WindowsFile.h
    Windows/WindowsLibrary.cpp
    Windows/WindowsLibrary.h
    Windows/WindowsMisc.cpp
    Windows/WindowsTime.cpp
    Windows/WindowsTime.h
    Windows/WindowsWindow.cpp
    Windows/WindowsWindow.h
)

include(CMakeCommon)

add_library(${HERMES_SUBLIB_NAME} OBJECT ${SOURCES})

target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_BUILD_ENGINE)
target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_GAME_NAME="Sandbox") # TODO : expose as an CMake option later

target_include_directories(${HERMES_SUBLIB_NAME} PUBLIC ${HERMES_SOURCE_DIR})
target_link_libraries(${HERMES_SUBLIB_NAME} PRIVATE Hermes_Core)

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
