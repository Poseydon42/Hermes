local build_dir = "../Build/Binaries/%{cfg.platform}-%{cfg.buildcfg}"
local intermediate_dir = "../Build/Intermediate/%{cfg.platform}-%{cfg.buildcfg}"

project "Sandbox"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"
    architecture "x86_64"
    exceptionhandling "Off"

    targetname "Sandbox"
    targetdir(build_dir)
    objdir (intermediate_dir)

    dependson { "Hermes" }
    libdirs { build_dir }
    files { "Source/**.h", "Source/**.cpp" }
    includedirs { "Source/", "../Hermes/Source", os.getenv("VULKAN_SDK").."/Include/", "../Hermes/ThirdParty/VulkanMemoryAllocator/include" }
    defines { "HERMES_BUILD_APPLICATION", "HERMES_GAME_NAME=\"Sandbox\"" }
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
        -- FIXME : links option does not seem to work correctly when you're linking to executable project, so currently I'm using this workaround --
        linkoptions { build_dir.."/Hermes.lib"}
        system "windows"
        defines { "HERMES_PLATFORM_WINDOWS" }
    filter "system:Windows"
        systemversion "latest"