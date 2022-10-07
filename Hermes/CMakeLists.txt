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

set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "Instead of throwing exceptions assert.")
set(SPIRV_CROSS_STATIC ON CACHE BOOL "Build the C and C++ API as static libraries.")
set(SPIRV_CROSS_SHARED OFF CACHE BOOL "Build the C API as a single shared library.")
set(SPIRV_CROSS_CLI OFF CACHE BOOL "Build the CLI binary. Requires SPIRV_CROSS_STATIC.")
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Enable SPIRV-Cross tests.")
set(SPIRV_CROSS_ENABLE_GLSL ON CACHE BOOL "Enable GLSL support.")
set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "Enable HLSL target support.")
set(SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "Enable MSL target support.")
set(SPIRV_CROSS_ENABLE_CPP ON CACHE BOOL "Enable C++ target support.")
set(SPIRV_CROSS_ENABLE_REFLECT OFF CACHE BOOL "Enable JSON reflection target support.")
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE BOOL "Enable C API wrapper support in static library.")
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "Enable util module support.")
add_subdirectory(ThirdParty/SPIRV-Cross)

include(CMakeCommon)

add_executable(Hermes WIN32 Source/HermesExecutable.cpp)
target_link_libraries(Hermes PRIVATE Hermes_ApplicationCore Hermes_AssetSystem Hermes_Core Hermes_Launch Hermes_Logging Hermes_Math Hermes_Platform Hermes_RenderingEngine Hermes_RenderInterface)
if(MSVC)
    target_link_options(Hermes PRIVATE "/OPT:NOREF") # To prevent MSVC from removing some symbols from import lib
endif()

set_target_properties(Hermes PROPERTIES ENABLE_EXPORTS TRUE)
add_compile_definitions(HERMES_GAME_NAME="Sandbox") # TODO : expose as an CMake option later

target_include_directories(Hermes INTERFACE ${HERMES_SOURCE_DIR})
