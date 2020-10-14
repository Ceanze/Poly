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

function generate_spirv()
	local path = "Poly/libs/glslang/SPIRV"

	project "spirv"
		kind "StaticLib"
		language "C++"
		location (path)

		targetdir (path .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (path .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		files
		{
			path .. "/**"
		}
end

function generate_glslang()
	local path = "Poly/libs/glslang"

	-- Generate the necessary libraries first
	-- generate_spirv()

	project "glslang"
		kind "StaticLib"
		language "C++"
		location (path)

		local buildInfoPy	= "%{prj.location}build_info.py"
		local buildInfoTmpl	= "%{prj.location}build_info.h.tmpl"
		local includeDir	= "%{prj.location}/include"
		local buildInfoH	= includeDir .. "/glslang/build_info.h"

		prebuildcommands
		{
			"python " .. buildInfoPy .. " %{prj.location} -i " .. buildInfoTmpl .. " -o " .. buildInfoH
		}

		targetdir (path .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (path .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		filter "system:windows"
			defines { "GLSLANG_OSINCLUDE_WIN32" }
		
		filter "system:linux OR system:macosx"
			defines { "GLSLANG_OSINCLUDE_UNIX" }
		filter {}

		includedirs
		{
			path,
			path .. "/glslang/Public",
			path .. "/glslang/Include",
			path .. "/glslang/MachineIndependent",
			path .. "/OGLCompilersDLL",
			path .. "/External/spirv-tools/include", -- TODO: This is only possible after the generate_sources.py has been used!
			includeDir
		}

		files
		{
			path .. "/glslang/Include/**",
			path .. "/glslang/OSDependent/Windows/**",
			path .. "/glslang/Public/**",
			path .. "/glslang/MachineIndependent/**",
			path .. "/SPIRV/**",
			path .. "/StandAlone/**",
			path .. "/OGLCompilersDLL/**"
		}

		excludes
		{
			path .. "/gtests/**",
			path .. "/Test/**",
			path .. "/hlsl/**"
		}

		filter "configurations:Debug"
			runtime "Debug"
			symbols "on"
			defines { "ENABLE_OPT=1" }

		filter "configurations:Release"
			runtime "Release"
			optimize "on"
			defines { "ENABLE_OPT=0" }
end

function generate_glfw()
	-- Note: {Workspace} does not work when building remote

	local path = "Poly/libs/glfw/"

	project "GLFW"
		kind "StaticLib"
		language "C"
		location (path)

		targetdir (path .. "bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (path .. "bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		files
		{
			path .. "include/GLFW/glfw3.h",
			path .. "include/GLFW/glfw3native.h",
			path .. "src/glfw_config.h",
			path .. "src/context.c",
			path .. "src/init.c",
			path .. "src/input.c",
			path .. "src/monitor.c",
			path .. "src/vulkan.c",
			path .. "src/window.c"
		}
		
		filter "system:windows"
			systemversion "latest"
			staticruntime "On"

			files
			{
				path .. "src/win32_init.c",
				path .. "src/win32_joystick.c",
				path .. "src/win32_monitor.c",
				path .. "src/win32_time.c",
				path .. "src/win32_thread.c",
				path .. "src/win32_window.c",
				path .. "src/wgl_context.c",
				path .. "src/egl_context.c",
				path .. "src/osmesa_context.c"
			}

			defines 
			{
				"_GLFW_WIN32",
				"_CRT_SECURE_NO_WARNINGS"
			}

		filter "configurations:Debug"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			runtime "Release"
			optimize "on"
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

-- include "Poly/libs/glfw"
-- Generate submodule projects
generate_glfw()
generate_glslang()
-- generate_spirv()

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
	cppdialect "C++latest"

	pchheader "polypch.h"
	pchsource "Poly/src/polypch.cpp"

	setDirs()
	srcFiles()

	links
	{
		"vulkan-1",
		"glfw",
		"glslang"
		-- "spirv"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/libs/spdlog/include",
		"%{prj.name}/libs/glfw/include",
		"%{prj.name}/libs/glm",
		"%{prj.name}/libs/VMA/src",
		"%{prj.name}/libs/stb_image",
		-- "%{prj.name}/libs/glslang/glslang/include"
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