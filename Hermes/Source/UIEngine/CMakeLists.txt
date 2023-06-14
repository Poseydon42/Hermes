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
    Widgets/Button.cpp
    Widgets/Button.h
    Widgets/Containers/Container.cpp
    Widgets/Containers/Container.h
    Widgets/Containers/HorizontalContainer.cpp
    Widgets/Containers/HorizontalContainer.h
    Widgets/Containers/VerticalContainer.cpp
    Widgets/Containers/VerticalContainer.h
    Widgets/Containers/ViewportContainer.cpp
    Widgets/Containers/ViewportContainer.h
    Widgets/Image.cpp
    Widgets/Image.h
    Widgets/Label.cpp
    Widgets/Label.h
    Widgets/Panel.cpp
    Widgets/Panel.h
    Widgets/Widget.cpp
    Widgets/Widget.h
)

add_engine_sublib(Hermes_UIEngine "${SOURCES}")

target_include_directories(Hermes_UIEngine PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(Hermes_UIEngine PRIVATE ${FREETYPE_LIBRARIES})
