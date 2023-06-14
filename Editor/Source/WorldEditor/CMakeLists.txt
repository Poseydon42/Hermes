cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    WorldEditorCamera.cpp
    WorldEditorCamera.h
    WorldEditorEntityList.cpp
    WorldEditorEntityList.h
    WorldEditorViewport.cpp
    WorldEditorViewport.h
)

add_editor_sublib(Editor_WorldEditor "${SOURCES}")
