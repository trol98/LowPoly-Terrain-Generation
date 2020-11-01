workspace "TerrainGen"
	architecture "x86"
	startproject "TerrainGen"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Dependencies/GLFW/include"
IncludeDir["GLEW"] = "%{wks.location}/Dependencies/GLEW/include"


project "TerrainGen"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")


	files
	{
		"%{wks.location}/TerrainGen/src/*.h",
		"%{wks.location}/TerrainGen/src/*.cpp",
		"%{wks.location}/TerrainGen/src/vendor/glm/glm/**.hpp",
		"%{wks.location}/TerrainGen/src/vendor/glm/glm/**.inl",
		"%{wks.location}/TerrainGen/src/vendor/noise/**.h",
		"%{wks.location}/TerrainGen/src/vendor/noise/**.cpp"
	}

	defines
	{
		"GLEW_STATIC"
	}

	includedirs
	{
		"vendor",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLEW}"
	}

	links
	{
		"GLFW",
		"Dependencies/GLEW/lib/Release/Win32/glew32s.lib",
		"opengl32.lib",
		"OpenGL32.lib",
		"User32.lib",
		"gdi32.lib",
		"Shell32.lib"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

group "Dependencies"
	--include "TerrainGen/src/vendor/premake"
	include "Dependencies/GLFW"
group ""

--include "TerrainGen"


