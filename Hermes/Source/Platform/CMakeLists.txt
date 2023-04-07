cmake_minimum_required(VERSION 3.24)

set(SOURCES
    GenericPlatform/PlatformDebug.h
    GenericPlatform/PlatformFile.h
    GenericPlatform/PlatformLibrary.h
    GenericPlatform/PlatformMisc.h
    GenericPlatform/PlatformTime.h
    GenericPlatform/PlatformWindow.h
    Windows/WindowsDebug.cpp
    Windows/WindowsFile.cpp
    Windows/WindowsFile.h
    Windows/WindowsLibrary.cpp
    Windows/WindowsLibrary.h
    Windows/WindowsMisc.cpp
    Windows/WindowsTime.cpp
    Windows/WindowsWindow.cpp
    Windows/WindowsWindow.h
)

include(HermesSublib)

add_hermes_sublib(Hermes_Platform "${SOURCES}")
target_link_libraries(Hermes_Platform PUBLIC Hermes_Core)
target_link_libraries(Hermes_Platform PRIVATE Hermes_ApplicationCore)
