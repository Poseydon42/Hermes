cmake_minimum_required(VERSION 3.24)

set(SOURCES
    LaunchWindows.cpp
)

include(HermesSublib)

add_hermes_sublib(Hermes_Launch "${SOURCES}")
target_link_libraries(Hermes_Launch PRIVATE Hermes_Core Hermes_ApplicationCore)
