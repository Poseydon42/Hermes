cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    Application.h
    GameLoop.cpp
    GameLoop.h
    InputEngine.cpp
    InputEngine.h
)

add_hermes_sublib(Hermes_ApplicationCore "${SOURCES}")
