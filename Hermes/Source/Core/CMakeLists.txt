cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    Compiler/CompilerMSVC.h
    Core.h
    Event/Event.h
    Event/EventQueue.cpp
    Event/EventQueue.h
    Misc/ArgsParser.cpp
    Misc/ArgsParser.h
    Misc/DefaultConstructors.h
    Misc/EnumClassOperators.h
    Misc/KeyCode.h
    Misc/NonCopyableMovable.h
    Misc/Timer.cpp
    Misc/Timer.h
    Misc/Version.h
    Profiling.h
    UTF8/UTF8Iterator.cpp
    UTF8/UTF8Iterator.h
    UTF8/UTF8Utils.cpp
    UTF8/UTF8Utils.h
)

add_engine_sublib(Hermes_Core "${SOURCES}")
