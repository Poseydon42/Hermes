cmake_minimum_required(VERSION 3.24)

set(SOURCES
    DebugLogDevice.h
    FileLogDevice.cpp
    FileLogDevice.h
    ILogDevice.h
    Logger.cpp
    Logger.h
)

include(HermesSublib)

add_hermes_sublib(Hermes_Logging "${SOURCES}")
target_link_libraries(Hermes_Logging PUBLIC Hermes_Core Hermes_Platform)
