workspace "Poly"
	architecture "x64"
	startproject "Poly"
	language "C++"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	filter "system:windows"
		defines
		{
			"POLY_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "POLY_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "POLY_RELEASE"
		runtime "Release"
		optimize "on"


OUTPUT_DIR = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

libraryDirs = {}
-- libraryDirs["GLFW"] = "Horizon/libs/GLFW"

-- Functions for repeating project info

-- Include all src files in the project
function srcFiles()
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
end

-- Set the target and object directories
function setDirs()
	targetdir ("bin/" .. OUTPUT_DIR .. "/%{prj.name}")
	objdir ("bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")
end


-- Projects
project "Poly"
	location "Poly"
	kind "WindowedApp"
	cppdialect "C++17"

	pchheader "polypch.h"
	pchsource "Poly/src/polypch.cpp"

	setDirs()
	srcFiles()

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/libs/spdlog/include",
		-- "%{libraryDirs.GLFW}/include"
	}

	libdirs
	{
		-- "%{libraryDirs.GLFW}/lib"
	}

	filter "system:windows"
		systemversion "latest"
		-- links { "opengl32" }