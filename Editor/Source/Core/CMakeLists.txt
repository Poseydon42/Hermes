cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    EditorApplication.cpp
    EditorApplication.h
)

add_editor_sublib(Editor_Core "${SOURCES}")
