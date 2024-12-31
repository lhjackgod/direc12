workspace "DirectX"
    architecture "x64"
    configurations {"Debug", "Release"}
    startproject "DirectX"

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "DirectX"
    kind "ConsoleApp"
    language "C++"
    location "DirectX"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    files
    {
        "{prj.name}/src/**.cpp",
        "{prj.name}/src/**.h"
    }
    filter "system:window"
        systemversion "latest"
    filter "configurations:Debug"
        symbols "on"
    filter "configurations:Release"
        optimize "on"
    