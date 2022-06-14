set(CMAKE_CXX_EXTENSIONS FALSE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_compile_definitions($<$<CONFIG:Debug>:HERMES_DEBUG>)
add_compile_definitions($<$<CONFIG:Release>:HERMES_RELEASE>)

if(${WIN32})
    add_compile_definitions(HERMES_PLATFORM_WINDOWS)
    add_compile_definitions(UNICODE _UNICODE)
    add_compile_options("/MP")
else()
    message(FATAL_ERROR "Platforms other than Windows are not supported yet")
endif()

function(KeepFolderStructure ProjectDirectory ProjectSources)
    source_group(TREE ${ProjectDirectory} FILES ${ProjectSources})
endfunction()

