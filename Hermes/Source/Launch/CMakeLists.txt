cmake_minimum_required(VERSION 3.24)

set(SOURCES
    LaunchWindows.cpp
)

include(HermesSublib)

add_hermes_sublib(Hermes_Launch "${SOURCES}")
