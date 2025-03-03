workspace "MultiMesh"
    architecture "x64"
    configurations{"Debug", "Release"}
    outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
project "MultiMesh"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir ("bin/" .. outputDir.. "/%{prj.name}")
    objdir ("bin-int/" .. outputDir .. "/%{prj.name}")
    files
    {
        "src/**.h",
        "src/**.cpp"
    }
    includedirs
    {
        "src"
    }
    