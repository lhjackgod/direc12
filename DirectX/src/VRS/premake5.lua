workspace "VRS"
    architecture "x64"
    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }
outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
project "VRS"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir ("bin/" .. outputDir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputDir .. "/%{prj.name}")
    files
    {
        "src/**.h",
        "src/**.cpp",
    }
    includedirs
    {
        "src"
    }
