cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    Asset.cpp
    Asset.h
    AssetHeaders.h
    AssetLoader.cpp
    AssetLoader.h
)

add_engine_sublib(Hermes_AssetSystem "${SOURCES}")
