local path = "../Poly/libs/SPIRV-Reflect/"

project "SPIRV-Reflect"
    kind "StaticLib"
    language "C++"
	cppdialect "c++20"

    location (_WORKING_DIR .. "/projects/%{prj.name}")

    targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
    objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

    files
    {
        path .. "spirv_reflect.h",
        path .. "spirv_reflect.cpp"
    }