cmake_minimum_required(VERSION 3.24)

set(HERMES_SUBLIB_NAME Hermes_Core)

set(SOURCES
    Compiler/CompilerMSVC.h
    Core.h
    Delegate/Delegate.h
    Event/Event.h
    Event/EventQueue.cpp
    Event/EventQueue.h
    Misc/DefaultConstructors.h
    Misc/EnumClassOperators.h
    Misc/NonCopyableMovable.h
    Misc/ArgsParser.cpp
    Misc/ArgsParser.h
    Misc/StringUtils.cpp
    Misc/StringUtils.h
    Profiling.h
)

include(CMakeCommon)

add_library(${HERMES_SUBLIB_NAME} OBJECT ${SOURCES})

target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_BUILD_ENGINE)
target_compile_definitions(${HERMES_SUBLIB_NAME} PRIVATE HERMES_GAME_NAME="Sandbox") # TODO : expose as an CMake option later

target_include_directories(${HERMES_SUBLIB_NAME} PUBLIC ${HERMES_SOURCE_DIR})
target_link_libraries(${HERMES_SUBLIB_NAME} PUBLIC $<$<CONFIG:Release>:Tracy::TracyClient>)
target_compile_definitions(${HERMES_SUBLIB_NAME} PUBLIC $<$<CONFIG:Release>:HERMES_ENABLE_PROFILING>)

# target_include_directories(${HERMES_SUBLIB_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
target_include_directories(${HERMES_SUBLIB_NAME} PUBLIC "${HERMES_THIRD_PARTY_DIR}/VulkanMemoryAllocator/include")
target_link_libraries(${HERMES_SUBLIB_NAME} PUBLIC Vulkan::Vulkan)

KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
