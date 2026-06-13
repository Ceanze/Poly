function get_vk_sdk_path()
	-- TODO: VULKAN_SDK should be enough - but for windows the VK_SDK_PATH might be needed - check if this is the case
	-- local sdkPathVars = {"VK_SDK_PATH", "VULKAN_SDK"}
	local sdkPathVars = {"VULKAN_SDK"}
	for _, sdkPathVar in ipairs(sdkPathVars) do
		sdkPath = os.getenv(sdkPathVar)
		if sdkPath ~= nil then
			return sdkPath
		end
	end

	print(string.format("No environment variables for path to Vulkan SDK are set: %s", sdkPathVars[0]))
	return ""
end

-- TODO: Add a proper search for this variable
function get_vulkan_include_dir(vkPath)
	return vkPath .. "/include"
end

function generate_glslang()
	include("premake/glslang.lua")
end

function generate_glfw()
	include("premake/glfw.lua")
end

function generate_assimp()
	include("premake/assimp.lua")
end

function generate_imgui()
	include("premake/imgui.lua")
end

function generate_yamlcpp()
	include("premake/yaml-cpp.lua")
end

function generate_spirv_reflect()
	include("premake/spirv-reflect.lua")
end

function generate_nvrhi()
	include("premake/nvrhi.lua")
end

vkPath = get_vk_sdk_path()
print(vkPath)

workspace "Poly"
	architecture "x64"
	startproject "Sandbox"
	language "C++"
	cppdialect "C++20"

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

	filter "system:macosx"
		defines
		{
			"POLY_PLATFORM_MACOS"
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
-- Generate submodule projects
generate_glfw()
generate_glslang()
generate_assimp()
generate_imgui()
generate_yamlcpp()
generate_spirv_reflect()
generate_nvrhi()

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
	cppdialect "C++20"

	filter "system:macosx"
	    links
	    {
	        "libvulkan"
	    }

	filter "system:windows"
	    links
	    {
	        "vulkan-1"
	    }
    filter {}

	links
	{
		"assimp",
		"glfw",
		"glslang",
		"SPIRV",
		"imgui",
		"yaml-cpp",
		"SPIRV-Reflect",
		"nvrhi",
		"nvrhi_vk"
	}

	includedirs
	{
		"%{prj.name}/src",
	}

	-- TODO: Check if just "polypch.h" is enough for windows too
	filter "system:windows"
		pchheader "polypch.h"
		pchsource "%{prj.name}/src/polypch.cpp"

	filter "action:not vs*"
		pchheader "src/polypch.h"

	filter "system:macosx"
		defines { "VK_USE_PLATFORM_MACOS_MVK" }
	filter {}

	setDirs()
	srcFiles()

	forceincludes
	{
		"polypch.h"
	}

	-- TODO: Check case sensitivity (Lib vs lib)
	libdirs
	{
		vkPath .. "/lib",
	}

	externalincludedirs
	{
		get_vulkan_include_dir(vkPath),
		"%{prj.name}/libs/NVRHI/include",
		"%{prj.name}/libs/spdlog/include",
		"%{prj.name}/libs/glfw/include",
		"%{prj.name}/libs/glm",
		"%{prj.name}/libs/VMA/include",
		"%{prj.name}/libs/stb_image",
		"%{prj.name}/libs/glslang",
		"%{prj.name}/libs/assimp/include",
		"%{prj.name}/libs/imgui",
		"%{prj.name}/libs/entt/src",
		"%{prj.name}/libs/yaml-cpp/include",
		"%{prj.name}/libs/SPIRV-Reflect"
	}

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	cppdialect "c++20"

	setDirs()
	srcFiles()

	externalincludedirs
	{
		"Poly/libs/glm",
		"Poly/src",
		"Poly/libs",
		"Poly/libs/entt/src",
		"Poly/libs/spdlog/include"
	}

	links
	{
		"Poly"
	}

	filter "system:macosx"
		links
		{
			"Cocoa.framework",
			"IOKit.framework",
			"CoreFoundation.framework",
			"Metal.framework",
			"IOSurface.framework",
			"QuartzCore.framework",
			"vulkan"
		}
		libdirs
		{
			vkPath .. "/lib",
		}
		runpathdirs
		{
			vkPath .. "/lib",
		}

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }
