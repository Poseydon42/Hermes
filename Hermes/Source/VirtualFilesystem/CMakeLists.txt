cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    DirectoryFSDevice.cpp
    DirectoryFSDevice.h
    FSDevice.h
    MountPoint.cpp
    MountPoint.h
    VirtualFilesystem.cpp
    VirtualFilesystem.h
)

add_engine_sublib(Hermes_VirtualFileSystem "${SOURCES}")
