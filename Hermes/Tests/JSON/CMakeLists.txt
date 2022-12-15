cmake_minimum_required(VERSION 3.24)

include(CMakeCommon)
include(TestExecutable)

project(Test_JSON)

set(SOURCES
    TestJSONObject.cpp
    TestJSONValue.cpp
)

add_test_executable(Test_JSON "${SOURCES}" Hermes_JSON)
KeepFolderStructure(${CMAKE_CURRENT_SOURCE_DIR} "${SOURCES}")
