local path = "../Poly/libs/yaml-cpp/"

project "yaml-cpp"
	kind "StaticLib"
	language "C++"
	location (_WORKING_DIR .. "/projects/%{prj.name}")

	targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
	objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

	files
	{
		path .. "include/**.h",
		path .. "src/**.h",
		path .. "src/**.cpp"
	}

	includedirs
	{
		path .. "include"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "off"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"