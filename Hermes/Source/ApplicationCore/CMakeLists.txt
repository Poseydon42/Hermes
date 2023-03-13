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
target_link_libraries(Hermes_ApplicationCore PUBLIC Hermes_Core Hermes_Math Hermes_Platform Hermes_RenderingEngine Hermes_World)
target_link_libraries(Hermes_ApplicationCore PRIVATE Hermes_Logging)
