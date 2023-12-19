function generate_prebuildcommands(path)
	-- This prebuildcommand needs to happen before all dependnecies in glslang
	-- Therefore it is called in the lowest denominator (GenericCodeGen)
	local buildInfoPy	= path .. "/build_info.py"
	local buildInfoTmpl	= path .. "/build_info.h.tmpl"
	local includeDir	= _WORKING_DIR .. "/projects/glslang/include"
	local buildInfoH	= includeDir .. "/glslang/build_info.h"

	filter "system:windows"
		prebuildcommands
		{
			"python " .. buildInfoPy .. " " .. path .. " -i " .. buildInfoTmpl .. " -o " .. buildInfoH
		}
	filter "system:macosx"
		prebuildcommands
		{
			"python3 " .. buildInfoPy .. " " .. path .. " -i " .. buildInfoTmpl .. " -o " .. buildInfoH
		}
	filter {}
end

function generate_osdeplib(libpath)
	project "OSDependent"
		kind "StaticLib"
		language "C++"
		location (_WORKING_DIR .. "/projects/%{prj.name}")

		targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		filter "system:windows"
			local root = libpath .. "/glslang/OSDependent/Windows"

			files
			{
				root .. "/ossource.cpp",
				root .. "/../osinclude.h"
			}

			defines { "GLSLANG_OSINCLUDE_WIN32" }

		filter "system:linux OR system:macosx"
			local root = libpath .. "/glslang/OSDependent/Unix"

			files
			{
				root .. "/ossource.cpp",
				root .. "/../osinclude.h"
			}

			defines { "GLSLANG_OSINCLUDE_UNIX" }
		filter {}
end

function generate_oglcompilerslib(libpath)
	local root = libpath .. "/OGLCompilersDLL"

	project "OGLCompiler"
		kind "StaticLib"
		language "C++"
		location (_WORKING_DIR .. "/projects/%{prj.name}")

		files
		{
			root .. "/InitializeDll.cpp",
			root .. "/InitializeDll.h"
		}

		filter "system:windows"
			defines { "GLSLANG_OSINCLUDE_WIN32" }

		filter "system:linux OR system:macosx"
			defines { "GLSLANG_OSINCLUDE_UNIX" }
		filter {}
end

function generate_glslanglib(libpath)
	generate_osdeplib(libpath)

	local root = libpath .. "/glslang"

	project "GenericCodeGen"
		kind "StaticLib"
		language "C++"
		cppdialect "C++20"
		location (_WORKING_DIR .. "/projects/%{prj.name}")

		generate_prebuildcommands(libpath)

		targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		files
		{
			root .. "/GenericCodeGen/Link.cpp",
			root .. "/GenericCodeGen/CodeGen.cpp",
		}

		filter "system:windows"
			defines { "GLSLANG_OSINCLUDE_WIN32" }

		filter "system:linux OR system:macosx"
			defines { "GLSLANG_OSINCLUDE_UNIX" }
		filter {}

	project "MachineIndependent"
		kind "StaticLib"
		language "C++"
		cppdialect "C++20"
		location (_WORKING_DIR .. "/projects/%{prj.name}")

		targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		includedirs
		{
			_WORKING_DIR .. "/projects/glslang/include"
		}

		files
		{
			root .. "/MachineIndependent/**.h",
			root .. "/MachineIndependent/**.cpp",
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
		cppdialect "C++20"
		location (_WORKING_DIR .. "/projects/%{prj.name}")

		targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		includedirs
		{
			libpath
		}

		files
		{
			root .. "/CInterface/glslang_c_interface.cpp",
			root .. "/Include/**.h",
			root .. "/Public/ShaderLang.h"
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
	local root = libpath .. "/SPIRV"

	project "SPIRV"
		kind "StaticLib"
		language "C++"
		cppdialect "C++20"
		location (_WORKING_DIR .. "/projects/%{prj.name}")

		targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		includedirs
		{
			libpath,
			_WORKING_DIR .. "/projects/glslang/include"
		}

		files
		{
			-- Sources
			root .. "/GlslangToSpv.cpp",
			root .. "/InReadableOrder.cpp",
			root .. "/Logger.cpp",
			root .. "/SpvBuilder.cpp",
			root .. "/SpvPostProcess.cpp",
			root .. "/doc.cpp",
			root .. "/SpvTools.cpp",
			root .. "/disassemble.cpp",
			root .. "/CInterface/spirv_c_interface.cpp",

			-- Headers
			root .. "/bitutils.h",
			root .. "/spirv.hpp",
			root .. "/GLSL.std.450.h",
			root .. "/GLSL.ext.EXT.h",
			root .. "/GLSL.ext.KHR.h",
			root .. "/GlslangToSpv.h",
			root .. "/hex_float.h",
			root .. "/Logger.h",
			root .. "/SpvBuilder.h",
			root .. "/spvIR.h",
			root .. "/doc.h",
			root .. "/SpvTools.h",
			root .. "/disassemble.h",
			root .. "/GLSL.ext.AMD.h",
			root .. "/GLSL.ext.NV.h",
			root .. "/NonSemanticDebugPrintf.h"
		}

		filter "system:windows"
			defines { "GLSLANG_OSINCLUDE_WIN32" }

		filter "system:linux OR system:macosx"
			defines { "GLSLANG_OSINCLUDE_UNIX" }
		filter {}
end

local path = ( _WORKING_DIR .. "/Poly/libs/glslang")

group "glslang"
	generate_glslanglib(path)
	generate_prebuildcommands(path)
	generate_oglcompilerslib(path)
	generate_spirvlib(path)

group ""