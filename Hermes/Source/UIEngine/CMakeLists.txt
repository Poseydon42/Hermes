cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    DrawingContext.cpp
    DrawingContext.h
    Window.cpp
    Window.h
    Widgets/PanelWidget.cpp
    Widgets/PanelWidget.h
    Widgets/Widget.cpp
    Widgets/Widget.h
)

add_hermes_sublib(Hermes_UIEngine "${SOURCES}")
target_link_libraries(Hermes_UIEngine PUBLIC Hermes_Core Hermes_Math)
target_link_libraries(Hermes_UIEngine PRIVATE Hermes_Logging Hermes_RenderingEngine)
