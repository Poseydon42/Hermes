include(CommonDefinitions)
include(GoogleTest)

function(add_test_executable test_name test_sources dependencies)
    project(${name} C CXX)

    add_executable(${test_name} ${test_sources})

    add_common_definitions(${test_name})

    target_compile_definitions(${test_name} PRIVATE HERMES_BUILD_TESTS)
    target_link_libraries(${test_name} ${dependencies} GTest::gtest_main)

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

    gtest_discover_tests(${test_name})
endfunction()
