cmake_minimum_required(VERSION 3.24)

include(HermesSublib)

set(SOURCES
    DrawingContext.cpp
    DrawingContext.h
    Font.cpp
    Font.h
    InputController.cpp
    InputController.h
    TextLayout.cpp
    TextLayout.h
    Widgets/Containers/ContainerWidget.cpp
    Widgets/Containers/ContainerWidget.h
    Widgets/Containers/VerticalContainerWidget.cpp
    Widgets/Containers/VerticalContainerWidget.h
    Widgets/Containers/ViewportContainerWidget.cpp
    Widgets/Containers/ViewportContainerWidget.h
    Widgets/LabelWidget.cpp
    Widgets/LabelWidget.h
    Widgets/PanelWidget.cpp
    Widgets/PanelWidget.h
    Widgets/Widget.cpp
    Widgets/Widget.h
)

add_hermes_sublib(Hermes_UIEngine "${SOURCES}")
target_link_libraries(Hermes_UIEngine PUBLIC Hermes_Core Hermes_Math)
target_link_libraries(Hermes_UIEngine PRIVATE Hermes_Logging Hermes_RenderingEngine)

target_include_directories(Hermes_UIEngine PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(Hermes_UIEngine PRIVATE ${FREETYPE_LIBRARIES})
