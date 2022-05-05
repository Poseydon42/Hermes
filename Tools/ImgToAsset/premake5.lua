local build_dir = "../Binaries"
local intermediate_dir = "../../Build/Intermediate/%{cfg.platform}-%{cfg.buildcfg}"

project "ImgToAsset"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    architecture "x86_64"
    exceptionhandling "On"

    targetname "ImgToAsset"
    targetdir(build_dir)
    objdir (intermediate_dir)

    files { "Source/**.h", "Source/**.cpp", "ThirdParty/*.h" }
    includedirs { "Source/", "ThirdParty/" }
    defines { "HERMES_BUILD_TOOLS" }
    flags { "FatalWarnings" }
    warnings "Extra"
 
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
        disablewarnings  { "4251", "26812", "26439" }
    filter "system:Windows"
        systemversion "latest"
