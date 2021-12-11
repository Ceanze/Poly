local path = "../Poly/libs/imgui/"

project "ImGui"
    kind "StaticLib"
    language "C++"
    location (path)

    targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
    objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

    files
    {
        path .. "imconfig.h",
        path .. "imgui.h",
        path .. "imgui.cpp",
        path .. "imgui_draw.cpp",
        path .. "imgui_internal.h",
        path .. "imgui_widgets.cpp",
        path .. "imstb_rectpack.h",
        path .. "imstb_textedit.h",
        path .. "imstb_truetype.h",
        path .. "imgui_demo.cpp",
        path .. "imgui_tables.cpp",
    }

    filter "system:windows"
        systemversion "latest"
        cppdialect "C++17"
        staticruntime "On"
	filter {}