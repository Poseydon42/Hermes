cmake_minimum_required(VERSION 3.24)

include(TestExecutable)

project(Test_Platform)

set(SOURCES
    TestPlatformFile.cpp
)

add_test_executable(Test_Platform "${SOURCES}" Hermes_Platform)
