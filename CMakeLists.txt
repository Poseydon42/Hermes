cmake_minimum_required(VERSION 3.24)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Meta)

set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)

project(HermesWorkspace)

set(CMAKE_CXX_EXTENSIONS FALSE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

option(HERMES_ENABLE_TESTING "Enable compiling and running tests for Hermes" ON)

set(HERMES_APPLICATION_TARGET Editor)
set(HERMES_APPLICATION_NAME "${HERMES_APPLICATION_TARGET}")
if(HERMES_APPLICATION_TARGET STREQUAL "Editor")
    set(HERMES_WITH_EDITOR 1)
else()
    set(HERMES_WITH_EDITOR 0)
endif()

if (NOT (${WIN32} EQUAL 1 AND ${MSVC} EQUAL 1))
    message(FATAL_ERROR "Only Windows builds using MSVC are supported at the moment")
endif()

if (WIN32)
    set(ZLIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/zlib)
endif()
set(ZLIB_USE_STATIC_LIBS ON)
find_package(ZLIB REQUIRED)

if(HERMES_ENABLE_TESTING)
    include(FetchContent)

    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()
endif()

add_subdirectory(Hermes)
add_subdirectory(Editor)
add_subdirectory(Sandbox)
add_subdirectory(Tools/ImgToAsset)
add_subdirectory(Tools/MeshToAsset)

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Hermes)
    set_property(TARGET Hermes PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Copy the application DLL into the folder with Hermes.exe
add_custom_target(CopyApplicationDLL ALL 
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${HERMES_APPLICATION_TARGET}> $<TARGET_FILE_DIR:Hermes>
    DEPENDS ${HERMES_APPLICATION_TARGET})
