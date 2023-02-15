cmake_minimum_required(VERSION 3.24)

include(TestExecutable)

project(Test_VirtualFilesystem)

set(SOURCES
    TestDirectoryFSDevice.cpp
    TestVirtualFilesystem.cpp
)

add_test_executable(Test_VirtualFilesystem "${SOURCES}" Hermes_VirtualFileSystem)
