cmake_minimum_required(VERSION 3.22)

project(Hermes)

set(HERMES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
set(HERMES_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty")

add_subdirectory(Source/ApplicationCore)
add_subdirectory(Source/AssetSystem)
add_subdirectory(Source/Core)
add_subdirectory(Source/Launch)
add_subdirectory(Source/Logging)
add_subdirectory(Source/Math)
add_subdirectory(Source/Platform)
add_subdirectory(Source/RenderingEngine)
add_subdirectory(Source/RenderInterface)

include(CMakeCommon)

add_executable(Hermes WIN32 Source/HermesExecutable.cpp)
target_link_libraries(Hermes PRIVATE Hermes_ApplicationCore Hermes_AssetSystem Hermes_Core Hermes_Launch Hermes_Logging Hermes_Math Hermes_Platform Hermes_RenderingEngine Hermes_RenderInterface)
if(MSVC)
    target_link_options(Hermes PRIVATE "/OPT:NOREF") # To prevent MSVC from removing some symbols from import lib
endif()

set_target_properties(Hermes PROPERTIES ENABLE_EXPORTS TRUE)
add_compile_definitions(HERMES_GAME_NAME="Sandbox") # TODO : expose as an CMake option later

target_include_directories(Hermes INTERFACE ${HERMES_SOURCE_DIR})
