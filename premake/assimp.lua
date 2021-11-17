function generate_config()
	-- CMake generates an .h file from a .h.in file and formats it depending
	-- on the value set. In assimp's config only one define is either 0 or 1
	-- change the variable beneth to set it to the corresponding value
	local ENABLE_DOUBLE_PRECISION = 1

	-- Read file to string
	local file = io.open(_WORKING_DIR .. "/Poly/libs/assimp/include/assimp/config.h.in", "r")
	local content = file:read("*all")
	file:close()

	-- Change #cmakedefine to #define
	content, _ = string.gsub(content, "#cmakedefine ASSIMP_DOUBLE_PRECISION 1", "//#define ASSIMP_DOUBLE_PRECISION " .. ENABLE_DOUBLE_PRECISION)

	-- Write to config.h file
	local new_file = io.open(_WORKING_DIR .. "/Poly/libs/assimp/include/assimp/config.h", "w")
	new_file:write(content)
	new_file:close()
end

function generate_revision()
	local file = io.open(_WORKING_DIR .. "/Poly/libs/assimp/revision.h.in", "r")
	local content = file:read("*all")
	file:close()

	content, _ = string.gsub(content, "@ASSIMP_VERSION_MAJOR@", 5)
	content, _ = string.gsub(content, "@ASSIMP_VERSION_MINOR@", 0)
	content, _ = string.gsub(content, "@ASSIMP_VERSION_PATCH@", 1)
	content, _ = string.gsub(content, "@ASSIMP_PACKAGE_VERSION@", 0)
	content, _ = string.gsub(content, "@GIT_COMMIT_HASH@", 0)
	content, _ = string.gsub(content, "@GIT_BRANCH@", "master")
	content, _ = string.gsub(content, "@LIBRARY_SUFFIX@", "")
	content, _ = string.gsub(content, "@CMAKE_DEBUG_POSTFIX@", "-d")

	local new_file = io.open(_WORKING_DIR .. "/Poly/libs/assimp/revision.h", "w")
	new_file:write(content)
	new_file:close()
end

project "assimp"
  kind "StaticLib"
  language "C++"
  cppdialect "C++17"
  location (_WORKING_DIR .. "/Poly/libs/assimp")

--   targetdir ("%{prj.location}/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
--   objdir ("%{prj.location}/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")
    targetdir (_WORKING_DIR .. "/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
    objdir (_WORKING_DIR .. "/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

  defines {
      -- "SWIG",
      "ASSIMP_BUILD_NO_OWN_ZLIB",

      "ASSIMP_BUILD_NO_X_IMPORTER",
      "ASSIMP_BUILD_NO_3DS_IMPORTER",
      "ASSIMP_BUILD_NO_MD3_IMPORTER",
      "ASSIMP_BUILD_NO_MDL_IMPORTER",
      "ASSIMP_BUILD_NO_MD2_IMPORTER",
      "ASSIMP_BUILD_NO_PLY_IMPORTER",
      "ASSIMP_BUILD_NO_ASE_IMPORTER",
      -- "ASSIMP_BUILD_NO_OBJ_IMPORTER",
      "ASSIMP_BUILD_NO_AMF_IMPORTER",
      "ASSIMP_BUILD_NO_HMP_IMPORTER",
      "ASSIMP_BUILD_NO_SMD_IMPORTER",
      "ASSIMP_BUILD_NO_MDC_IMPORTER",
      "ASSIMP_BUILD_NO_MD5_IMPORTER",
      "ASSIMP_BUILD_NO_STL_IMPORTER",
      "ASSIMP_BUILD_NO_LWO_IMPORTER",
      "ASSIMP_BUILD_NO_DXF_IMPORTER",
      "ASSIMP_BUILD_NO_NFF_IMPORTER",
      "ASSIMP_BUILD_NO_RAW_IMPORTER",
      "ASSIMP_BUILD_NO_OFF_IMPORTER",
      "ASSIMP_BUILD_NO_AC_IMPORTER",
      "ASSIMP_BUILD_NO_BVH_IMPORTER",
      "ASSIMP_BUILD_NO_IRRMESH_IMPORTER",
      "ASSIMP_BUILD_NO_IRR_IMPORTER",
      "ASSIMP_BUILD_NO_Q3D_IMPORTER",
      "ASSIMP_BUILD_NO_B3D_IMPORTER",
      "ASSIMP_BUILD_NO_COLLADA_IMPORTER",
      "ASSIMP_BUILD_NO_TERRAGEN_IMPORTER",
      "ASSIMP_BUILD_NO_CSM_IMPORTER",
      "ASSIMP_BUILD_NO_3D_IMPORTER",
      "ASSIMP_BUILD_NO_LWS_IMPORTER",
      "ASSIMP_BUILD_NO_OGRE_IMPORTER",
      "ASSIMP_BUILD_NO_OPENGEX_IMPORTER",
      "ASSIMP_BUILD_NO_MS3D_IMPORTER",
      "ASSIMP_BUILD_NO_M3D_IMPORTER",
      "ASSIMP_BUILD_NO_COB_IMPORTER",
      "ASSIMP_BUILD_NO_BLEND_IMPORTER",
      "ASSIMP_BUILD_NO_Q3BSP_IMPORTER",
      "ASSIMP_BUILD_NO_NDO_IMPORTER",
      "ASSIMP_BUILD_NO_IFC_IMPORTER",
      "ASSIMP_BUILD_NO_XGL_IMPORTER",
      "ASSIMP_BUILD_NO_FBX_IMPORTER",
      "ASSIMP_BUILD_NO_ASSBIN_IMPORTER",
    --   "ASSIMP_BUILD_NO_GLTF_IMPORTER",
      "ASSIMP_BUILD_NO_C4D_IMPORTER",
      "ASSIMP_BUILD_NO_3MF_IMPORTER",
      "ASSIMP_BUILD_NO_X3D_IMPORTER",
      "ASSIMP_BUILD_NO_MMD_IMPORTER",
      
      "ASSIMP_BUILD_NO_STEP_EXPORTER",
      "ASSIMP_BUILD_NO_SIB_IMPORTER",

      -- "ASSIMP_BUILD_NO_MAKELEFTHANDED_PROCESS",
      -- "ASSIMP_BUILD_NO_FLIPUVS_PROCESS",
      -- "ASSIMP_BUILD_NO_FLIPWINDINGORDER_PROCESS",
      -- "ASSIMP_BUILD_NO_CALCTANGENTS_PROCESS",
      "ASSIMP_BUILD_NO_JOINVERTICES_PROCESS",
      -- "ASSIMP_BUILD_NO_TRIANGULATE_PROCESS",
      "ASSIMP_BUILD_NO_GENFACENORMALS_PROCESS",
      -- "ASSIMP_BUILD_NO_GENVERTEXNORMALS_PROCESS",
      "ASSIMP_BUILD_NO_REMOVEVC_PROCESS",
      "ASSIMP_BUILD_NO_SPLITLARGEMESHES_PROCESS",
      "ASSIMP_BUILD_NO_PRETRANSFORMVERTICES_PROCESS",
      "ASSIMP_BUILD_NO_LIMITBONEWEIGHTS_PROCESS",
      -- "ASSIMP_BUILD_NO_VALIDATEDS_PROCESS",
      "ASSIMP_BUILD_NO_IMPROVECACHELOCALITY_PROCESS",
      "ASSIMP_BUILD_NO_FIXINFACINGNORMALS_PROCESS",
      "ASSIMP_BUILD_NO_REMOVE_REDUNDANTMATERIALS_PROCESS",
      "ASSIMP_BUILD_NO_FINDINVALIDDATA_PROCESS",
      "ASSIMP_BUILD_NO_FINDDEGENERATES_PROCESS",
      "ASSIMP_BUILD_NO_SORTBYPTYPE_PROCESS",
      "ASSIMP_BUILD_NO_GENUVCOORDS_PROCESS",
      "ASSIMP_BUILD_NO_TRANSFORMTEXCOORDS_PROCESS",
      "ASSIMP_BUILD_NO_FINDINSTANCES_PROCESS",
      "ASSIMP_BUILD_NO_OPTIMIZEMESHES_PROCESS",
      "ASSIMP_BUILD_NO_OPTIMIZEGRAPH_PROCESS",
      "ASSIMP_BUILD_NO_SPLITBYBONECOUNT_PROCESS",
      "ASSIMP_BUILD_NO_DEBONE_PROCESS",
      "ASSIMP_BUILD_NO_EMBEDTEXTURES_PROCESS",
      "ASSIMP_BUILD_NO_GLOBALSCALE_PROCESS",
      "RAPIDJSON_HAS_STDSTRING=1"
  }

  files {
      "%{prj.location}/include/**",
      "%{prj.location}/code/Common/Assimp.cpp",
      "%{prj.location}/code/Common/BaseImporter.cpp",
      "%{prj.location}/code/Common/CreateAnimMesh.cpp",
      "%{prj.location}/code/Common/BaseProcess.cpp",
      "%{prj.location}/code/PostProcessing/EmbedTexturesProcess.cpp",
      "%{prj.location}/code/PostProcessing/ConvertToLHProcess.cpp",
      "%{prj.location}/code/Common/DefaultIOStream.cpp",
      "%{prj.location}/code/Common/DefaultIOSystem.cpp",
      "%{prj.location}/code/Common/DefaultLogger.cpp",
      "%{prj.location}/code/PostProcessing/GenVertexNormalsProcess.cpp",
      "%{prj.location}/code/Common/Importer.cpp",
      "%{prj.location}/code/Common/ImporterRegistry.cpp",
      "%{prj.location}/code/Material/MaterialSystem.cpp",
      "%{prj.location}/code/Common/PostStepRegistry.cpp",
      "%{prj.location}/code/PostProcessing/ProcessHelper.cpp",
      "%{prj.location}/code/Common/scene.cpp",
      "%{prj.location}/code/Common/ScenePreprocessor.cpp",
      "%{prj.location}/code/PostProcessing/ScaleProcess.cpp",
      "%{prj.location}/code/Common/SGSpatialSort.cpp",
      "%{prj.location}/code/Common/SkeletonMeshBuilder.cpp",
      "%{prj.location}/code/Common/SpatialSort.cpp",
      "%{prj.location}/code/PostProcessing/TriangulateProcess.cpp",
      "%{prj.location}/code/PostProcessing/ValidateDataStructure.cpp",
      "%{prj.location}/code/Common/Version.cpp",
      "%{prj.location}/code/Common/VertexTriangleAdjacency.cpp",
      "%{prj.location}/code/PostProcessing/MakeVerboseFormat.cpp",
      "%{prj.location}/code/PostProcessing/CalcTangentsProcess.cpp",
      "%{prj.location}/code/PostProcessing/ScaleProcess.cpp",
      "%{prj.location}/code/PostProcessing/EmbedTexturesProcess.cpp",
      "%{prj.location}/code/PostProcessing/GenBoundingBoxesProcess.cpp",
      "%{prj.location}/code/PostProcessing/ArmaturePopulate.cpp",
	  "%{prj.location}/code/Common/Exceptional.cpp",
	  "%{prj.location}/code/Common/AssertHandler.cpp",
	  "%{prj.location}/code/Common/material.cpp",
	  "%{prj.location}/contrib/irrXML/*",

	  -- Custom
	  "%{prj.location}/code/AssetLib/Obj/**",
	  "%{prj.location}/code/AssetLib/glTF/**",
	  "%{prj.location}/code/AssetLib/glTF2/**",
  }

  includedirs {
      "%{prj.location}",
      "%{prj.location}/include",
      "%{prj.location}/code",
      "%{prj.location}/contrib",
      "%{prj.location}/contrib/irrXML",
      "%{prj.location}/contrib/zlib",
      "%{prj.location}/contrib/pugixml/src",
      "%{prj.location}/contrib/rapidjson/include",
  }


   filter "system:windows"
      systemversion "latest"

   filter  "configurations:Debug"
       runtime "Debug"
       symbols "on"

   filter  "configurations:Release"
       runtime "Release"
	   optimize "on"

generate_revision()
generate_config()