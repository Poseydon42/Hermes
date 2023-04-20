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

add_hermes_sublib(Hermes_JSON "${SOURCES}")
target_link_libraries(Hermes_JSON PUBLIC Hermes_Core)
target_link_libraries(Hermes_JSON PRIVATE Hermes_Logging Hermes_Math)
