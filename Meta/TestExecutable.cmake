function(add_test_executable test_name test_sources dependencies)
    include(GoogleTest)

    add_executable(${test_name} ${test_sources})
    target_compile_definitions(${test_name} PRIVATE HERMES_BUILD_TESTS)
    target_link_libraries(${test_name} ${dependencies} GTest::gtest_main)

    gtest_discover_tests(${test_name})
endfunction()
