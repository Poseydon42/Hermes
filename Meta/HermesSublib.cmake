include(CommonDefinitions)

function(add_hermes_sublib name sources)
    project(${name} CXX)

    add_library(${name} STATIC ${sources})
    add_common_definitions(${name})

    target_compile_definitions(${name} PRIVATE $<$<CONFIG:Release>:HERMES_ENABLE_PROFILING>)
    target_compile_definitions(${name} PRIVATE HERMES_APPLICATION_NAME="${HERMES_APPLICATION_NAME}")
    target_compile_definitions(${name} PRIVATE $<${HERMES_WITH_EDITOR}:HERMES_WITH_EDITOR>)

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})
endfunction()

function(add_engine_sublib name sources)
    add_hermes_sublib(${name} "${sources}")

    target_compile_definitions(${name} PRIVATE HERMES_BUILD_ENGINE)
    target_include_directories(${name} PUBLIC ${HERMES_SOURCE_DIR})

    # FIXME: this is a bit ugly :(
    target_link_libraries(${name} PUBLIC $<$<CONFIG:Release>:Tracy::TracyClient>)
    target_include_directories(${name} PRIVATE "${HERMES_THIRD_PARTY_DIR}/VulkanMemoryAllocator/include")
    target_link_libraries(${name} PRIVATE Vulkan::Vulkan)
endfunction()

function(add_editor_sublib name sources)
    add_hermes_sublib(${name} "${sources}")

    target_compile_definitions(${name} PRIVATE HERMES_BUILD_APPLICATION)
    target_include_directories(${name} PUBLIC ${HERMES_EDITOR_DIR})
    target_link_libraries(${name} PRIVATE Hermes)
endfunction()
