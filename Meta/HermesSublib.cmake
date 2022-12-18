include(CommonDefinitions)

function(add_hermes_sublib name sources)
    project(${name} C CXX)

    add_library(${name} STATIC ${sources})
    add_common_definitions(${name})

    target_compile_definitions(${name} PRIVATE HERMES_BUILD_ENGINE)
    target_compile_definitions(${name} PRIVATE HERMES_GAME_NAME="${HERMES_GAME_NAME}")

    target_include_directories(${name} PUBLIC ${HERMES_SOURCE_DIR})

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})
endfunction()
