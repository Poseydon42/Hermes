cmake_minimum_required(VERSION 3.24)

include(TestExecutable)

project(Test_JSON)

set(SOURCES
    TestJSONObject.cpp
    TestJSONParser.cpp
    TestJSONValue.cpp
)

add_test_executable(Test_JSON "${SOURCES}" Hermes_JSON)
