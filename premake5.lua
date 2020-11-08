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

function generate_osdeplib(libpath)
	project "OSDependent"
		kind "StaticLib"
		language "C++"

		filter "system:windows"
			location (libpath .. "/glslang/OSDependent/Windows")

			targetdir ("%{prj.location}/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
			objdir ("%{prj.location}/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

			files
			{
				"%{prj.location}/ossource.cpp",
				"%{prj.location}/../osinclude.h"
			}

			defines { "GLSLANG_OSINCLUDE_WIN32" }
	
		filter "system:linux OR system:macosx"
			location (libpath .. "/OSDependent/Unix")

			targetdir ("%{prj.location}/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
			objdir ("%{prj.location}/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

			files
			{
				"%{prj.location}/ossource.cpp",
				"%{prj.location}/../osinclude.h"
			}

			defines { "GLSLANG_OSINCLUDE_UNIX" }
		filter {}
end

function generate_oglcompilerslib(libpath)
	project "OGLCompiler"
		kind "StaticLib"
		language "C++"
		location (libpath .. "/OGLCompilersDLL")

		files
		{
			"%{prj.location}/InitializeDll.cpp",
			"%{prj.location}/InitializeDll.h"
		}

		filter "system:windows"
			defines { "GLSLANG_OSINCLUDE_WIN32" }
	
		filter "system:linux OR system:macosx"
			defines { "GLSLANG_OSINCLUDE_UNIX" }
		filter {}
end

function generate_glslanglib(libpath)
	generate_osdeplib(libpath)

	project "GenericCodeGen"
		kind "StaticLib"
		language "C++"
		location (libpath .. "/glslang")

		targetdir ("%{prj.location}/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir ("%{prj.location}/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		files
		{
			"%{prj.location}/GenericCodeGen/Link.cpp",
			"%{prj.location}/GenericCodeGen/CodeGen.cpp",
		}

		filter "system:windows"
			defines { "GLSLANG_OSINCLUDE_WIN32" }
	
		filter "system:linux OR system:macosx"
			defines { "GLSLANG_OSINCLUDE_UNIX" }
		filter {}

	project "MachineIndependent"
		kind "StaticLib"
		language "C++"
		location (libpath .. "/glslang")

		targetdir ("%{prj.location}/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir ("%{prj.location}/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		includedirs
		{
			libpath .. "/include"
		}

		files
		{
			"%{prj.location}/MachineIndependent/**.h",
			"%{prj.location}/MachineIndependent/**.cpp",
		}

		links
		{
			"OGLCompiler",
			"GenericCodeGen",
			"OSDependent"
		}

		filter "system:windows"
			defines { "GLSLANG_OSINCLUDE_WIN32" }
	
		filter "system:linux OR system:macosx"
			defines { "GLSLANG_OSINCLUDE_UNIX" }
		filter {}

	project "glslang"
		kind "StaticLib"
		language "C++"
		location (libpath .. "/glslang")
		
		targetdir ("%{prj.location}/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir ("%{prj.location}/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		includedirs
		{
			libpath
		}

		files
		{
			"%{prj.location}/CInterface/glslang_c_interface.cpp",
			"%{prj.location}/Include/**.h",
			"%{prj.location}/Public/ShaderLang.h"
		}

		links
		{
			"OGLCompiler",
			"OSDependent",
			"MachineIndependent"
		}

		filter "system:windows"
			defines { "GLSLANG_OSINCLUDE_WIN32" }
	
		filter "system:linux OR system:macosx"
			defines { "GLSLANG_OSINCLUDE_UNIX" }
		filter {}
end

function generate_spirvlib(libpath)
	project "SPIRV"
		kind "StaticLib"
		language "C++"
		location (libpath .. "/SPIRV")

		targetdir ("%{prj.location}/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir ("%{prj.location}/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		includedirs
		{
			libpath,
			libpath .. "/include"
		}

		files
		{
			-- Sources
			"%{prj.location}/GlslangToSpv.cpp",
			"%{prj.location}/InReadableOrder.cpp",
			"%{prj.location}/Logger.cpp",
			"%{prj.location}/SpvBuilder.cpp",
			"%{prj.location}/SpvPostProcess.cpp",
			"%{prj.location}/doc.cpp",
			"%{prj.location}/SpvTools.cpp",
			"%{prj.location}/disassemble.cpp",
			"%{prj.location}/CInterface/spirv_c_interface.cpp",

			-- Headers
			"%{prj.location}/bitutils.h",
			"%{prj.location}/spirv.hpp",
			"%{prj.location}/GLSL.std.450.h",
			"%{prj.location}/GLSL.ext.EXT.h",
			"%{prj.location}/GLSL.ext.KHR.h",
			"%{prj.location}/GlslangToSpv.h",
			"%{prj.location}/hex_float.h",
			"%{prj.location}/Logger.h",
			"%{prj.location}/SpvBuilder.h",
			"%{prj.location}/spvIR.h",
			"%{prj.location}/doc.h",
			"%{prj.location}/SpvTools.h",
			"%{prj.location}/disassemble.h",
			"%{prj.location}/GLSL.ext.AMD.h",
			"%{prj.location}/GLSL.ext.NV.h",
			"%{prj.location}/NonSemanticDebugPrintf.h"
		}

		filter "system:windows"
			defines { "GLSLANG_OSINCLUDE_WIN32" }
	
		filter "system:linux OR system:macosx"
			defines { "GLSLANG_OSINCLUDE_UNIX" }
		filter {}
end

function generate_glslang()
	local path = ( _WORKING_DIR .. "/Poly/libs/glslang")

	group "glslang"
		generate_glslanglib(path)
		generate_oglcompilerslib(path)
		generate_spirvlib(path)

		local buildInfoPy	= path .. "/build_info.py"
		local buildInfoTmpl	= path .. "/build_info.h.tmpl"
		local includeDir	= path .. "/include"
		local buildInfoH	= includeDir .. "/glslang/build_info.h"

		prebuildcommands
		{
			"python " .. buildInfoPy .. " " .. path .. " -i " .. buildInfoTmpl .. " -o " .. buildInfoH
		}

	group ""
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
-- Generate submodule projects
generate_glfw()
generate_glslang()

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
		"glslang",
		"SPIRV"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/libs/spdlog/include",
		"%{prj.name}/libs/glfw/include",
		"%{prj.name}/libs/glm",
		"%{prj.name}/libs/VMA/src",
		"%{prj.name}/libs/stb_image",
		"%{prj.name}/libs/glslang"
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