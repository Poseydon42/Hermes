cmake_minimum_required(VERSION 3.24)

include(TestExecutable)

project(Test_Core)

set(SOURCES
    TestUTF8Iterator.cpp
    TestUTF8Utils.cpp
    TestVersion.cpp
)

add_test_executable(Test_Core "${SOURCES}" Hermes_Core)
