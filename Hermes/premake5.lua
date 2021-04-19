local build_dir = "../Build/Binaries/%{cfg.platform}-%{cfg.buildcfg}"
local intermediate_dir = "../Build/Intermediate/%{cfg.platform}-%{cfg.buildcfg}"

project "Hermes"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++17"
    architecture "x86_64"
    exceptionhandling "Off"

    targetname "Hermes"
    targetdir(build_dir)
    objdir (intermediate_dir)

    files { "Source/**.h", "Source/**.cpp" }
    includedirs { "Source/", os.getenv("VULKAN_SDK").."/Include/" }
    libdirs { os.getenv("VULKAN_SDK").."/Lib/" } -- 32 bit builds are not supported --
    defines { "HERMES_BUILD_ENGINE", "HERMES_GAME_NAME=\"Sandbox\"" }
    links { "vulkan-1" }

    filter "configurations:Debug"
        defines { "HERMES_DEBUG" }
        warnings "Extra"
        intrinsics "Off"
        symbols "On"
        optimize "Off"
        runtime "Debug"
    filter "configurations:Development"
        defines { "HERMES_DEVELOPMENT", "HERMES_INTRINSICS" }
        intrinsics "On"
        symbols "On"
        optimize "On"
        runtime "Debug"
    filter "configurations:Release"
        defines { "HERMES_RELEASE", "HERMES_INTRINSICS" }
        intrinsics "On"
        symbols "Off"
        optimize "On"
        runtime "Release"

    filter "platforms:Windows"
        system "windows"
        defines { "HERMES_PLATFORM_WINDOWS" }
        disablewarnings "4251"
    filter "system:Windows"
        systemversion "latest"