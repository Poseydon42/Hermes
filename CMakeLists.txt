cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Meta)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

project(HermesWorkspace)

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
add_custom_target(CopyGameDLL ALL)

add_custom_command(TARGET CopyGameDLL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:Sandbox> $<TARGET_FILE_DIR:Hermes>
    DEPENDS Sandbox)
