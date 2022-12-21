cmake_minimum_required(VERSION 3.24)

include(TestExecutable)

project(Test_Math)

set(SOURCES
    TestFromString.cpp
)

add_test_executable(Test_Math "${SOURCES}" Hermes_Math)
