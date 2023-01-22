cmake_minimum_required(VERSION 3.24)

include(TestExecutable)

project(Test_Core)

set(SOURCES
    TestVersion.cpp
)

add_test_executable(Test_Core "${SOURCES}" Hermes_Core)
