set(CMAKE_CXX_EXTENSIONS FALSE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_compile_definitions($<$<CONFIG:Debug>:HERMES_DEBUG>)
add_compile_definitions($<$<CONFIG:Release>:HERMES_RELEASE>)

if(${WIN32})
    add_compile_definitions(HERMES_PLATFORM_WINDOWS)
    add_compile_definitions(UNICODE _UNICODE)
    add_compile_options("/MP")
    add_compile_options(/W4 /WX) # Enable all warnings and treat them as errors
    add_compile_options(/wd4251) # And disable warning C4251(... needs to have dll-interface to be used by clients of class ...)
else()
    message(FATAL_ERROR "Platforms other than Windows are not supported yet")
endif()

function(KeepFolderStructure ProjectDirectory ProjectSources)
    source_group(TREE ${ProjectDirectory} FILES ${ProjectSources})
endfunction()

