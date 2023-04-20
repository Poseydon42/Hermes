cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    Asset.cpp
    Asset.h
    AssetHeaders.h
    AssetLoader.cpp
    AssetLoader.h
)

add_hermes_sublib(Hermes_AssetSystem "${SOURCES}")
target_link_libraries(Hermes_AssetSystem PUBLIC Hermes_Core Hermes_Math)
target_link_libraries(Hermes_AssetSystem PRIVATE Hermes_Logging Hermes_Platform)
