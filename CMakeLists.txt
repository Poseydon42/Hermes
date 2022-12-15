cmake_minimum_required(VERSION 3.24)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Meta)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

if (WIN32)
    set(ZLIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/zlib)
endif()
set(ZLIB_USE_STATIC_LIBS ON)
find_package(ZLIB REQUIRED)

project(HermesWorkspace)

option(HERMES_ENABLE_TESTING "Enable compiling and running tests for Hermes" ON)

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
add_subdirectory(Sandbox)
add_subdirectory(Tools/ImgToAsset)
add_subdirectory(Tools/MeshToAsset)

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Hermes)
    set_property(TARGET Hermes PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
    message(FATAL_ERROR "Compilers other than MSVC are not supported yet")
endif()

# Copy Sandbox.dll into the folder with Hermes.exe
add_custom_target(CopyGameDLL ALL 
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Sandbox> $<TARGET_FILE_DIR:Hermes>
    DEPENDS Sandbox)
