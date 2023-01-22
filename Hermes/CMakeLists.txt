cmake_minimum_required(VERSION 3.22)

project(Hermes C CXX)

set(HERMES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
set(HERMES_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty")

find_package(Vulkan REQUIRED)

add_subdirectory(Source/ApplicationCore)
add_subdirectory(Source/AssetSystem)
add_subdirectory(Source/Core)
add_subdirectory(Source/JSON)
add_subdirectory(Source/Launch)
add_subdirectory(Source/Logging)
add_subdirectory(Source/Math)
add_subdirectory(Source/Platform)
add_subdirectory(Source/RenderingEngine)
add_subdirectory(Source/Vulkan)

if(HERMES_ENABLE_TESTING)
    add_subdirectory(Tests/Core)
    add_subdirectory(Tests/JSON)
    add_subdirectory(Tests/Math)
    add_subdirectory(Tests/Platform)
endif()

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

set(TRACY_STATIC OFF CACHE BOOL "Build Tracy as a dynamic library because we need to use it from both the engine and the game DLL")
add_subdirectory(ThirdParty/tracy)
# Copy tracy.dll into the folder where Hermes.exe is located
add_custom_target(CopyTracyDLL ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:TracyClient> $<TARGET_FILE_DIR:Hermes>
    DEPENDS TracyClient)

set(HERMES_SUBLIB_LIST
    Hermes_ApplicationCore
    Hermes_AssetSystem
    Hermes_Core
    Hermes_JSON
    Hermes_Launch
    Hermes_Logging
    Hermes_Math
    Hermes_Platform
    Hermes_RenderingEngine
    Hermes_Vulkan
)

include(CommonDefinitions)

add_executable(Hermes WIN32 Source/HermesExecutable.cpp)
add_common_definitions(Hermes)
target_include_directories(Hermes PUBLIC ${HERMES_SOURCE_DIR})
target_link_libraries(Hermes PRIVATE ${HERMES_SUBLIB_LIST})

# FIXME: remove this once the game code will not have to include anything that itself includes Vulkan headers
target_include_directories(Hermes PUBLIC "${HERMES_THIRD_PARTY_DIR}/VulkanMemoryAllocator/include")
target_link_libraries(Hermes PUBLIC Vulkan::Vulkan)

set_target_properties(Hermes PROPERTIES ENABLE_EXPORTS TRUE)
if(MSVC)
    # NOTE: this prevents MSVC from omitting symbols that are not used in the engine itself in the import lib
    # FIXME: this feels hacky, can we do better?
    target_link_options(Hermes PRIVATE "/OPT:NOREF")
    foreach(SUBLIB IN LISTS HERMES_SUBLIB_LIST)
        target_link_options(Hermes PRIVATE "/WHOLEARCHIVE:${SUBLIB}.lib")
    endforeach(SUBLIB in ${HERMES_SUBLIB_LIST})
endif()
