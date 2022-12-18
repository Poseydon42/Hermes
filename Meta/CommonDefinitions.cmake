function(add_common_definitions target_name) 
    target_compile_definitions(${target_name} PRIVATE $<$<CONFIG:Debug>:HERMES_DEBUG>)
    target_compile_definitions(${target_name} PRIVATE $<$<CONFIG:Release>:HERMES_RELEASE>)

    if (WIN32)
        target_compile_definitions(${target_name} PRIVATE HERMES_PLATFORM_WINDOWS)
        target_compile_definitions(${target_name} PRIVATE UNICODE _UNICODE)
        target_compile_definitions(${target_name} PRIVATE NOMINMAX)
        target_compile_definitions(${target_name} PRIVATE WIN32_LEAN_AND_MEAN)
        target_compile_options(${target_name} PRIVATE "/MP")
        target_compile_options(${target_name} PRIVATE /W4 /WX) # Enable all warnings and treat them as errors
        target_compile_options(${target_name} PRIVATE /wd4251) # And disable warning C4251(... needs to have dll-interface to be used by clients of class ...)
    endif()
endfunction()
