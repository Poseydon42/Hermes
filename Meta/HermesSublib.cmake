include(CommonDefinitions)

function(add_hermes_sublib name sources)
    project(${name} C CXX)

    add_library(${name} STATIC ${sources})
    add_common_definitions(${name})

    target_compile_definitions(${name} PRIVATE $<$<CONFIG:Release>:HERMES_ENABLE_PROFILING>)
    target_compile_definitions(${name} PRIVATE HERMES_BUILD_ENGINE)
    target_compile_definitions(${name} PRIVATE HERMES_APPLICATION_NAME="${HERMES_GAME_NAME}")

    target_include_directories(${name} PUBLIC ${HERMES_SOURCE_DIR})

    # FIXME: this is a bit ugly :(
    target_link_libraries(${name} PUBLIC $<$<CONFIG:Release>:Tracy::TracyClient>)
    target_include_directories(${name} PRIVATE "${HERMES_THIRD_PARTY_DIR}/VulkanMemoryAllocator/include")
    target_link_libraries(${name} PRIVATE Vulkan::Vulkan)

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})
endfunction()
