local path = "../Poly/libs/glfw/"

project "GLFW"
	kind "StaticLib"
	language "C"
	location (_WORKING_DIR .. "/projects/%{prj.name}")

    targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
    objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

	files
	{
		path .. "include/GLFW/glfw3.h",
		path .. "include/GLFW/glfw3native.h",
		-- path .. "src/glfw_config.h",
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

	filter "system:macosx"
		staticruntime "On"

		files
		{
			path .. "src/cocoa_init.m",
			path .. "src/cocoa_joystick.m",
			path .. "src/cocoa_monitor.m",
			path .. "src/cocoa_time.c",
			path .. "src/posix_thread.c",
			path .. "src/cocoa_window.m",
			path .. "src/nsgl_context.m",
			path .. "src/egl_context.c",
			path .. "src/osmesa_context.c",
		}

		links
		{
			"-framework Cocoa",
			"-framework IOKit",
			"-framework CoreFoundation"
		}

		defines
		{
			"_GLFW_COCOA"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"