local path = "../Poly/libs/NVRHI/"

group "NVRHI"

project "nvrhi"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	location (_WORKING_DIR .. "/projects/%{prj.name}")
	targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
	objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

	externalincludedirs
	{
		path .. "include"
	}

	files
	{
		-- Common headers
		path .. "include/nvrhi/nvrhi.h",
		path .. "include/nvrhi/nvrhiHLSL.h",
		path .. "include/nvrhi/utils.h",
		path .. "include/nvrhi/common/containers.h",
		path .. "include/nvrhi/common/misc.h",
		path .. "include/nvrhi/common/resource.h",
		path .. "include/nvrhi/common/aftermath.h",

		-- Common sources
		path .. "src/common/format-info.cpp",
		path .. "src/common/misc.cpp",
		path .. "src/common/state-tracking.cpp",
		path .. "src/common/state-tracking.h",
		path .. "src/common/utils.cpp",
		path .. "src/common/aftermath.cpp",

		-- Validation layer
		path .. "include/nvrhi/validation.h",
		path .. "src/validation/validation-commandlist.cpp",
		path .. "src/validation/validation-device.cpp",
		path .. "src/validation/validation-backend.h",
	}

	defines
	{
		"NVRHI_WITH_AFTERMATH=0"
	}

	filter "system:windows"
		systemversion "latest"
		files { path .. "tools/nvrhi.natvis" }
	filter {}

project "nvrhi_vk"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	location (_WORKING_DIR .. "/projects/%{prj.name}")
	targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
	objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

	externalincludedirs
	{
		path .. "include",
		get_vulkan_include_dir(vkPath)
	}

	files
	{
		path .. "include/nvrhi/vulkan.h",
		path .. "src/common/versioning.h",
		path .. "src/vulkan/vulkan-allocator.cpp",
		path .. "src/vulkan/vulkan-backend.h",
		path .. "src/vulkan/vulkan-buffer.cpp",
		path .. "src/vulkan/vulkan-commandlist.cpp",
		path .. "src/vulkan/vulkan-compute.cpp",
		path .. "src/vulkan/vulkan-constants.cpp",
		path .. "src/vulkan/vulkan-device.cpp",
		path .. "src/vulkan/vulkan-graphics.cpp",
		path .. "src/vulkan/vulkan-meshlets.cpp",
		path .. "src/vulkan/vulkan-queries.cpp",
		path .. "src/vulkan/vulkan-queue.cpp",
		path .. "src/vulkan/vulkan-raytracing.cpp",
		path .. "src/vulkan/vulkan-resource-bindings.cpp",
		path .. "src/vulkan/vulkan-shader.cpp",
		path .. "src/vulkan/vulkan-staging-texture.cpp",
		path .. "src/vulkan/vulkan-state-tracking.cpp",
		path .. "src/vulkan/vulkan-texture.cpp",
		path .. "src/vulkan/vulkan-upload.cpp",
	}

	defines
	{
		"NVRHI_WITH_AFTERMATH=0"
	}

	filter "system:windows"
		systemversion "latest"
		defines
		{
			"VK_USE_PLATFORM_WIN32_KHR",
			"NOMINMAX"
		}
	filter {}

group ""
