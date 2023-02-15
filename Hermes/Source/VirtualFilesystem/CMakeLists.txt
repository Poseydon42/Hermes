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

add_hermes_sublib(Hermes_VirtualFileSystem "${SOURCES}")
target_link_libraries(Hermes_VirtualFileSystem PUBLIC Hermes_Core)
target_link_libraries(Hermes_VirtualFileSystem PRIVATE Hermes_Logging Hermes_Platform)
