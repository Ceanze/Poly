function generate_osdeplib(libpath)
	project "OSDependent"
		kind "StaticLib"
		language "C++"

		filter "system:windows"
			location (libpath .. "/glslang/OSDependent/Windows")

			targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
			objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

			files
			{
				"%{prj.location}/ossource.cpp",
				"%{prj.location}/../osinclude.h"
			}

			defines { "GLSLANG_OSINCLUDE_WIN32" }
	
		filter "system:linux OR system:macosx"
			location (libpath .. "/OSDependent/Unix")

			targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
			objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

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

		targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

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

		targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

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
		
		targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

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

		targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

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