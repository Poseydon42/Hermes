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

add_engine_sublib(Hermes_Logging "${SOURCES}")
