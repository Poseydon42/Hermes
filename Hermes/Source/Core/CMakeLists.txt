cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    Compiler/CompilerMSVC.h
    Core.h
    Event/Event.h
    Event/EventQueue.cpp
    Event/EventQueue.h
    Misc/DefaultConstructors.h
    Misc/EnumClassOperators.h
    Misc/KeyCode.h
    Misc/NonCopyableMovable.h
    Misc/ArgsParser.cpp
    Misc/ArgsParser.h
    Misc/UTF8StringView.cpp
    Misc/UTF8StringView.h
    Misc/Version.h
    Profiling.h
)

add_hermes_sublib(Hermes_Core "${SOURCES}")
target_link_libraries(Hermes_Core PUBLIC $<$<CONFIG:Release>:Tracy::TracyClient>)
target_compile_definitions(Hermes_Core PUBLIC $<$<CONFIG:Release>:HERMES_ENABLE_PROFILING>)
