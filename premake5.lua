function get_vk_sdk_path()
	-- These are SDK Path Environment Variables that are set on windows when Vulkan installs
	local sdkPathVars = {"VK_SDK_PATH", "VULKAN_SDK"}
	for _, sdkPathVar in ipairs(sdkPathVars) do
		sdkPath = os.getenv(sdkPathVar)
		if sdkPath ~= nil then
			return sdkPath
		end
	end

	print(string.format("No environment variables for path to Vulkan SDK are set: %s", array_to_string(sdkPathVars)))
	return ""
end

vkPath = get_vk_sdk_path()

workspace "Poly"
	architecture "x64"
	startproject "Sandbox"
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

-- Functions for repeating project info

include "Poly/libs/glfw"

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
	kind "StaticLib"
	cppdialect "C++17"

	pchheader "polypch.h"
	pchsource "Poly/src/polypch.cpp"

	setDirs()
	srcFiles()

	links
	{
		"vulkan-1",
		"glfw"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/libs/spdlog/include",
		"%{prj.name}/libs/glfw/include",
		"%{prj.name}/libs/glm",
		"%{prj.name}/libs/VMA/src",
		"%{prj.name}/libs/stb_image"
	}

	libdirs
	{
		vkPath .. "/Lib"
	}

	sysincludedirs
	{
		vkPath .. "/Include",
	}

	filter "system:windows"
		systemversion "latest"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	cppdialect "c++latest"

	setDirs()
	srcFiles()

	includedirs
	{
		"Poly/libs/spdlog/include",
		"Poly/libs/glm",
		"Poly/src",
		"Poly/libs"
	}

	links
	{
		"Poly"
	}

	filter "system:windows"
		systemversion "latest"