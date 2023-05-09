cmake_minimum_required(VERSION 3.24)

set(SOURCES
    LaunchWindows.cpp
)

include(HermesSublib)

add_engine_sublib(Hermes_Launch "${SOURCES}")
