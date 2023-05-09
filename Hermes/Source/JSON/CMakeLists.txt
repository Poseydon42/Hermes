cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    JSONObject.cpp
    JSONObject.h
    JSONParser.cpp
    JSONParser.h
    JSONValue.cpp
    JSONValue.h
)

add_engine_sublib(Hermes_JSON "${SOURCES}")
