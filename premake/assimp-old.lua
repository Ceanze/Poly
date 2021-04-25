function generate_config(path)
	-- CMake generates an .h file from a .h.in file and formats it depending
	-- on the value set. In assimp's config only one define is either 0 or 1
	-- change the variable beneth to set it to the corresponding value
	local ENABLE_DOUBLE_PRECISION = 1

	-- Read file to string
	local file = io.open(path .. "/include/assimp/config.h.in", "r")
	local content = file:read("*all")
	file:close()

	-- Change #cmakedefine to #define
	content, _ = string.gsub(content, "#cmakedefine ASSIMP_DOUBLE_PRECISION 1", "#define ASSIMP_DOUBLE_PRECISION " .. ENABLE_DOUBLE_PRECISION)

	-- Write to config.h file
	local new_file = io.open(path .. "/include/assimp/config.h", "w")
	new_file:write(content)
	new_file:close()
end

function generate_zconfig(path)
	-- Should only need to create a .h file - no substition is neded
	local file = io.open(path .. "/contrib/zlib/zconf.h.in", "r")
	local content = file:read("*all")
	file:close()
	local new_file = io.open(path .. "/contrib/zlib/zconf.h", "w")
	new_file:write(content)
	new_file:close()
end

function generate_zlibpc(path)
	-- Should only need to create a .h file - no substition is neded
	local file = io.open(path .. "/contrib/zlib/zlib.pc.cmakein", "r")
	local content = file:read("*all")
	file:close()
	local new_file = io.open(path .. "/contrib/zlib/zlib.pc", "w")
	new_file:write(content)
	new_file:close()
end

function generate_revision(path)
	local file = io.open(path .. "/revision.h.in", "r")
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

	local new_file = io.open(path .. "/revision.h", "w")
	new_file:write(content)
	new_file:close()
end

project_path = (_WORKING_DIR .. "/Poly/libs/assimp")

generate_config(project_path)
generate_zconfig(project_path)
generate_revision(project_path)
generate_zlibpc(project_path)

function generate_zlib()
	project "zlib"
		kind "StaticLib"
		warnings "Off"
		optimize "Speed"
		language "C"
		location (project_path .. "/contrib/zlib")

		targetdir ("%{prj.location}/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir ("%{prj.location}/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		includedirs
		{
			"%{prj.location}",
			project_path
		}

		files
		{
			"%{prj.location}/zconf.h",
			"%{prj.location}/zlib.h",
			"%{prj.location}/crc32.h",
			"%{prj.location}/deflate.h",
			"%{prj.location}/gzguts.h",
			"%{prj.location}/inffast.h",
			"%{prj.location}/inffixed.h",
			"%{prj.location}/inflate.h",
			"%{prj.location}/inftrees.h",
			"%{prj.location}/trees.h",
			"%{prj.location}/zutil.h",
			"%{prj.location}/adler32.c",
			"%{prj.location}/compress.c",
			"%{prj.location}/crc32.c",
			"%{prj.location}/deflate.c",
			"%{prj.location}/gzclose.c",
			"%{prj.location}/gzlib.c",
			"%{prj.location}/gzread.c",
			"%{prj.location}/gzwrite.c",
			"%{prj.location}/inflate.c",
			"%{prj.location}/infback.c",
			"%{prj.location}/inftrees.c",
			"%{prj.location}/inffast.c",
			"%{prj.location}/trees.c",
			"%{prj.location}/uncompr.c",
			"%{prj.location}/zutil.c",
			"%{prj.location}/win32/zlib1.rc" -- maybe
		}

		defines
		{
			"_CRT_SECURE_NO_DEPRECATE",
			"_CRT_NONSTDC_NO_DEPRECATE"
		}

		filter "configurations:Debug"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			runtime "Release"
			optimize "on"

end

function generate_openddl_parser()
	project "openddl_parser"
		kind "StaticLib"
		warnings "Off"
		optimize "Speed"
		language "C++"
		location (project_path .. "/contrib/openddlparser")

		targetdir ("%{prj.location}/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir ("%{prj.location}/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		includedirs
		{
			"%{prj.location}",
			"%{prj.location}/include",
			"%{prj.location}/"
			-- project_path .. "/contrib/gtest/",
			-- project_path .. "/contrib/gtest/include",
		}

		files
		{
			"%{prj.location}/code/OpenDDLCommon.cpp",
			"%{prj.location}/code/OpenDDLExport.cpp",
			"%{prj.location}/code/OpenDDLParser.cpp",
			"%{prj.location}/code/OpenDDLStream.cpp",
			"%{prj.location}/code/DDLNode.cpp",
			"%{prj.location}/code/Value.cpp",
			"%{prj.location}/include/openddlparser/OpenDDLCommon.h",
			"%{prj.location}/include/openddlparser/OpenDDLExport.h",
			"%{prj.location}/include/openddlparser/OpenDDLParser.h",
			"%{prj.location}/include/openddlparser/OpenDDLParserUtils.h",
			"%{prj.location}/include/openddlparser/OpenDDLStream.h",
			"%{prj.location}/include/openddlparser/DDLNode.h",
			"%{prj.location}/include/openddlparser/Value.h",
			"%{prj.location}/include/openddlparser/TPoolAllocator.h",
			"%{prj.location}/README.md"
		}

		defines
		{
			"_CRT_SECURE_NO_WARNINGS",
			"OPENDDL_STATIC_LIBARY",
			"GTEST_HAS_TR1_TUPLE=0",
			"OPENDDLPARSER_BUILD",
			"_VARIADIC_MAX=10",
			"GTEST_HAS_PTHREAD=0"
		}

end

function generate_assimp()
	project "assimp"
		kind "StaticLib"
		warnings "Off"
		optimize "Speed"
		language "C++"
		location (project_path .. "/code")

		targetdir ("%{prj.location}/bin/" .. OUTPUT_DIR .. "/%{prj.name}")
		objdir ("%{prj.location}/bin-int/" .. OUTPUT_DIR .. "/%{prj.name}")

		local header_path = "%{prj.location}/../include/assimp"

		-- links { "zlib", "openddl_parser" }

		flags
		{
			"MultiProcessorCompile"
		}

		includedirs
		{
			"%{prj.location}",
			"%{prj.location}/../contrib/unzip",
			"%{prj.location}/../contrib/zlib",
			"%{prj.location}/../contrib/openddlparser/include",
			project_path,
			header_path .. "/..",

			-- RapidJSON
			"%{prj.location}/../contrib/rapidjson/include",
			"%{prj.location}/../contrib",
			"%{prj.location}/../contrib/pugixml/src"
		}

		files
		{
			-- Compiler headers
			header_path .. "/Compiler/pushpack1.h",
			header_path .. "/Compiler/poppack1.h",
			header_path .. "/Compiler/pstdint.h",

			-- zlib special
			"%{prj.location}/../contrib/zlib/zlib.h",

			-- Public headers
			header_path .. "/anim.h",
			header_path .. "/aabb.h",
			header_path .. "/ai_assert.h",
			header_path .. "/camera.h",
			header_path .. "/color4.h",
			header_path .. "/color4.inl",
			header_path .. "/config.h",
			header_path .. "/ColladaMetaData.h",
			header_path .. "/commonMetaData.h",
			header_path .. "/defs.h",
			header_path .. "/Defines.h",
			header_path .. "/cfileio.h",
			header_path .. "/light.h",
			header_path .. "/material.h",
			header_path .. "/material.inl",
			header_path .. "/matrix3x3.h",
			header_path .. "/matrix3x3.inl",
			header_path .. "/matrix4x4.h",
			header_path .. "/matrix4x4.inl",
			header_path .. "/mesh.h",
			header_path .. "/pbrmaterial.h",
			header_path .. "/postprocess.h",
			header_path .. "/quaternion.h",
			header_path .. "/quaternion.inl",
			header_path .. "/scene.h",
			header_path .. "/metadata.h",
			header_path .. "/texture.h",
			header_path .. "/types.h",
			header_path .. "/vector2.h",
			header_path .. "/vector2.inl",
			header_path .. "/vector3.h",
			header_path .. "/vector3.inl",
			header_path .. "/version.h",
			header_path .. "/cimport.h",
			header_path .. "/importerdesc.h",
			header_path .. "/Importer.hpp",
			header_path .. "/DefaultLogger.hpp",
			header_path .. "/ProgressHandler.hpp",
			header_path .. "/IOStream.hpp",
			header_path .. "/IOSystem.hpp",
			header_path .. "/Logger.hpp",
			header_path .. "/LogStream.hpp",
			header_path .. "/NullLogger.hpp",
			header_path .. "/cexport.h",
			header_path .. "/Exporter.hpp",
			header_path .. "/DefaultIOStream.h",
			header_path .. "/DefaultIOSystem.h",
			header_path .. "/ZipArchiveIOSystem.h",
			header_path .. "/SceneCombiner.h",
			header_path .. "/fast_atof.h",
			header_path .. "/qnan.h",
			header_path .. "/BaseImporter.h",
			header_path .. "/Hash.h",
			header_path .. "/MemoryIOWrapper.h",
			header_path .. "/ParsingUtils.h",
			header_path .. "/StreamReader.h",
			header_path .. "/StreamWriter.h",
			header_path .. "/StringComparison.h",
			header_path .. "/StringUtils.h",
			header_path .. "/SGSpatialSort.h",
			header_path .. "/GenericProperty.h",
			header_path .. "/SpatialSort.h",
			header_path .. "/SkeletonMeshBuilder.h",
			header_path .. "/SmallVector.h",
			header_path .. "/SmoothingGroups.h",
			header_path .. "/SmoothingGroups.inl",
			header_path .. "/StandardShapes.h",
			header_path .. "/RemoveComments.h",
			header_path .. "/Subdivision.h",
			header_path .. "/Vertex.h",
			header_path .. "/LineSplitter.h",
			header_path .. "/TinyFormatter.h",
			header_path .. "/Profiler.h",
			header_path .. "/LogAux.h",
			header_path .. "/Bitmap.h",
			header_path .. "/XMLTools.h",
			header_path .. "/IOStreamBuffer.h",
			header_path .. "/CreateAnimMesh.h",
			header_path .. "/XmlParser.h",
			header_path .. "/BlobIOSystem.h",
			header_path .. "/MathFunctions.h",
			header_path .. "/Exceptional.h",
			header_path .. "/ByteSwapper.h",

			-- Core_SRCS
			"%{prj.location}/Common/Assimp.cpp",
			"%{prj.location}/res/assimp.rc",

			-- Logging_SRCS
			header_path .. "/DefaultLogger.hpp",
			header_path .. "/LogStream.hpp",
			header_path .. "/Logger.hpp",
			header_path .. "/NullLogger.hpp",
			"%{prj.location}/Common/Win32DebugLogStream.h",
			"%{prj.location}/Common/DefaultLogger.cpp",
			"%{prj.location}/Common/FileLogStream.h",
			"%{prj.location}/Common/StdOStreamLogStream.h",

			-- Common_SRCS
			"%{prj.location}/Common/BaseImporter.cpp",
			"%{prj.location}/Common/BaseProcess.cpp",
			"%{prj.location}/Common/BaseProcess.h",
			"%{prj.location}/Common/Importer.h",
			"%{prj.location}/Common/ScenePrivate.h",
			"%{prj.location}/Common/PostStepRegistry.cpp",
			"%{prj.location}/Common/ImporterRegistry.cpp",
			"%{prj.location}/Common/DefaultProgressHandler.h",
			"%{prj.location}/Common/DefaultIOStream.cpp",
			"%{prj.location}/Common/DefaultIOSystem.cpp",
			"%{prj.location}/Common/ZipArchiveIOSystem.cpp",
			"%{prj.location}/Common/PolyTools.h",
			"%{prj.location}/Common/Importer.cpp",
			"%{prj.location}/Common/IFF.h",
			"%{prj.location}/Common/SGSpatialSort.cpp",
			"%{prj.location}/Common/VertexTriangleAdjacency.cpp",
			"%{prj.location}/Common/VertexTriangleAdjacency.h",
			"%{prj.location}/Common/SpatialSort.cpp",
			"%{prj.location}/Common/SceneCombiner.cpp",
			"%{prj.location}/Common/ScenePreprocessor.cpp",
			"%{prj.location}/Common/ScenePreprocessor.h",
			"%{prj.location}/Common/SkeletonMeshBuilder.cpp",
			"%{prj.location}/Common/StandardShapes.cpp",
			"%{prj.location}/Common/TargetAnimation.cpp",
			"%{prj.location}/Common/TargetAnimation.h",
			"%{prj.location}/Common/RemoveComments.cpp",
			"%{prj.location}/Common/Subdivision.cpp",
			"%{prj.location}/Common/scene.cpp",
			"%{prj.location}/Common/Bitmap.cpp",
			"%{prj.location}/Common/Version.cpp",
			"%{prj.location}/Common/CreateAnimMesh.cpp",
			"%{prj.location}/Common/simd.h",
			"%{prj.location}/Common/simd.cpp",
			"%{prj.location}/Common/material.cpp",
			"%{prj.location}/Common/AssertHandler.cpp",
			"%{prj.location}/Common/Exceptional.cpp",

			-- CApi_SRCS
			"%{prj.location}/CApi/CInterfaceIOWrapper.cpp",
			"%{prj.location}/CApi/CInterfaceIOWrapper.h",

			-- STETPParser_SRCS
			"%{prj.location}/AssetLib/STEPParser/STEPFileReader.h",
			"%{prj.location}/AssetLib/STEPParser/STEPFileReader.cpp",
			"%{prj.location}/AssetLib/STEPParser/STEPFileEncoding.cpp",
			"%{prj.location}/AssetLib/STEPParser/STEPFileEncoding.h",

			-- PostProcessing_SRCS
			"%{prj.location}/PostProcessing/CalcTangentsProcess.cpp",
			"%{prj.location}/PostProcessing/CalcTangentsProcess.h",
			"%{prj.location}/PostProcessing/ComputeUVMappingProcess.cpp",
			"%{prj.location}/PostProcessing/ComputeUVMappingProcess.h",
			"%{prj.location}/PostProcessing/ConvertToLHProcess.cpp",
			"%{prj.location}/PostProcessing/ConvertToLHProcess.h",
			"%{prj.location}/PostProcessing/EmbedTexturesProcess.cpp",
			"%{prj.location}/PostProcessing/EmbedTexturesProcess.h",
			"%{prj.location}/PostProcessing/FindDegenerates.cpp",
			"%{prj.location}/PostProcessing/FindDegenerates.h",
			"%{prj.location}/PostProcessing/FindInstancesProcess.cpp",
			"%{prj.location}/PostProcessing/FindInstancesProcess.h",
			"%{prj.location}/PostProcessing/FindInvalidDataProcess.cpp",
			"%{prj.location}/PostProcessing/FindInvalidDataProcess.h",
			"%{prj.location}/PostProcessing/FixNormalsStep.cpp",
			"%{prj.location}/PostProcessing/FixNormalsStep.h",
			"%{prj.location}/PostProcessing/DropFaceNormalsProcess.cpp",
			"%{prj.location}/PostProcessing/DropFaceNormalsProcess.h",
			"%{prj.location}/PostProcessing/GenFaceNormalsProcess.cpp",
			"%{prj.location}/PostProcessing/GenFaceNormalsProcess.h",
			"%{prj.location}/PostProcessing/GenVertexNormalsProcess.cpp",
			"%{prj.location}/PostProcessing/GenVertexNormalsProcess.h",
			"%{prj.location}/PostProcessing/PretransformVertices.cpp",
			"%{prj.location}/PostProcessing/PretransformVertices.h",
			"%{prj.location}/PostProcessing/ImproveCacheLocality.cpp",
			"%{prj.location}/PostProcessing/ImproveCacheLocality.h",
			"%{prj.location}/PostProcessing/JoinVerticesProcess.cpp",
			"%{prj.location}/PostProcessing/JoinVerticesProcess.h",
			"%{prj.location}/PostProcessing/LimitBoneWeightsProcess.cpp",
			"%{prj.location}/PostProcessing/LimitBoneWeightsProcess.h",
			"%{prj.location}/PostProcessing/RemoveRedundantMaterials.cpp",
			"%{prj.location}/PostProcessing/RemoveRedundantMaterials.h",
			"%{prj.location}/PostProcessing/RemoveVCProcess.cpp",
			"%{prj.location}/PostProcessing/RemoveVCProcess.h",
			"%{prj.location}/PostProcessing/SortByPTypeProcess.cpp",
			"%{prj.location}/PostProcessing/SortByPTypeProcess.h",
			"%{prj.location}/PostProcessing/SplitLargeMeshes.cpp",
			"%{prj.location}/PostProcessing/SplitLargeMeshes.h",
			"%{prj.location}/PostProcessing/TextureTransform.cpp",
			"%{prj.location}/PostProcessing/TextureTransform.h",
			"%{prj.location}/PostProcessing/TriangulateProcess.cpp",
			"%{prj.location}/PostProcessing/TriangulateProcess.h",
			"%{prj.location}/PostProcessing/ValidateDataStructure.cpp",
			"%{prj.location}/PostProcessing/ValidateDataStructure.h",
			"%{prj.location}/PostProcessing/OptimizeGraph.cpp",
			"%{prj.location}/PostProcessing/OptimizeGraph.h",
			"%{prj.location}/PostProcessing/OptimizeMeshes.cpp",
			"%{prj.location}/PostProcessing/OptimizeMeshes.h",
			"%{prj.location}/PostProcessing/DeboneProcess.cpp",
			"%{prj.location}/PostProcessing/DeboneProcess.h",
			"%{prj.location}/PostProcessing/ProcessHelper.h",
			"%{prj.location}/PostProcessing/ProcessHelper.cpp",
			"%{prj.location}/PostProcessing/MakeVerboseFormat.cpp",
			"%{prj.location}/PostProcessing/MakeVerboseFormat.h",
			"%{prj.location}/PostProcessing/ScaleProcess.cpp",
			"%{prj.location}/PostProcessing/ScaleProcess.h",
			"%{prj.location}/PostProcessing/ArmaturePopulate.cpp",
			"%{prj.location}/PostProcessing/ArmaturePopulate.h",
			"%{prj.location}/PostProcessing/GenBoundingBoxesProcess.cpp",
			"%{prj.location}/PostProcessing/GenBoundingBoxesProcess.h",
			"%{prj.location}/PostProcessing/SplitByBoneCountProcess.cpp",
			"%{prj.location}/PostProcessing/SplitByBoneCountProcess.h",

			-- pugixml
			"%{prj.location}/../contrib/pugixml/src/pugiconfig.hpp",
			"%{prj.location}/../contrib/pugixml/src/pugixml.hpp",

			-- utf8
			"%{prj.location}/../contrib/utfcpp/source/utf8.h",

			-- Polyclipping
			"%{prj.location}/../contrib/clipper/clipper.hpp",
			"%{prj.location}/../contrib/clipper/clipper.cpp",

			-- Poly2tri
			"%{prj.location}/../contrib/poly2tri/poly2tri/common/shapes.cc",
			"%{prj.location}/../contrib/poly2tri/poly2tri/common/shapes.h",
			"%{prj.location}/../contrib/poly2tri/poly2tri/common/utils.h",
			"%{prj.location}/../contrib/poly2tri/poly2tri/sweep/advancing_front.h",
			"%{prj.location}/../contrib/poly2tri/poly2tri/sweep/advancing_front.cc",
			"%{prj.location}/../contrib/poly2tri/poly2tri/sweep/cdt.cc",
			"%{prj.location}/../contrib/poly2tri/poly2tri/sweep/cdt.h",
			"%{prj.location}/../contrib/poly2tri/poly2tri/sweep/sweep.cc",
			"%{prj.location}/../contrib/poly2tri/poly2tri/sweep/sweep.h",
			"%{prj.location}/../contrib/poly2tri/poly2tri/sweep/sweep_context.cc",
			"%{prj.location}/../contrib/poly2tri/poly2tri/sweep/sweep_context.h",

			-- minizip/unzip
			"%{prj.location}/../contrib/unzip/crypt.c",
			"%{prj.location}/../contrib/unzip/crypt.h",
			"%{prj.location}/../contrib/unzip/ioapi.c",
			"%{prj.location}/../contrib/unzip/ioapi.h",
			"%{prj.location}/../contrib/unzip/unzip.c",
			"%{prj.location}/../contrib/unzip/unzip.h",

			-- ziplip
			"%{prj.location}/../contrib/zip/src/miniz.h",
			"%{prj.location}/../contrib/zip/src/zip.c",
			"%{prj.location}/../contrib/zip/src/zip.h",

			-- openddlparser
			"%{prj.location}/../contrib/openddlparser/code/OpenDDLParser.cpp",
			"%{prj.location}/../contrib/openddlparser/code/DDLNode.cpp",
			"%{prj.location}/../contrib/openddlparser/code/OpenDDLCommon.cpp",
			"%{prj.location}/../contrib/openddlparser/code/OpenDDLExport.cpp",
			"%{prj.location}/../contrib/openddlparser/code/Value.cpp",
			"%{prj.location}/../contrib/openddlparser/code/OpenDDLStream.cpp",
			"%{prj.location}/../contrib/openddlparser/include/openddlparser/OpenDDLParser.h",
			"%{prj.location}/../contrib/openddlparser/include/openddlparser/OpenDDLParserUtils.h",
			"%{prj.location}/../contrib/openddlparser/include/openddlparser/OpenDDLCommon.h",
			"%{prj.location}/../contrib/openddlparser/include/openddlparser/OpenDDLExport.h",
			"%{prj.location}/../contrib/openddlparser/include/openddlparser/OpenDDLStream.h",
			"%{prj.location}/../contrib/openddlparser/include/openddlparser/DDLNode.h",
			"%{prj.location}/../contrib/openddlparser/include/openddlparser/Value.h",

			-- Open3dgc
			"%{prj.location}/../contrib/Open3DGC/o3dgcAdjacencyInfo.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcArithmeticCodec.cpp",
			"%{prj.location}/../contrib/Open3DGC/o3dgcArithmeticCodec.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcBinaryStream.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcCommon.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcDVEncodeParams.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcDynamicVectorDecoder.cpp",
			"%{prj.location}/../contrib/Open3DGC/o3dgcDynamicVectorDecoder.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcDynamicVectorEncoder.cpp",
			"%{prj.location}/../contrib/Open3DGC/o3dgcDynamicVectorEncoder.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcDynamicVector.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcFIFO.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcIndexedFaceSet.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcIndexedFaceSet.inl",
			"%{prj.location}/../contrib/Open3DGC/o3dgcSC3DMCDecoder.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcSC3DMCDecoder.inl",
			"%{prj.location}/../contrib/Open3DGC/o3dgcSC3DMCEncodeParams.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcSC3DMCEncoder.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcSC3DMCEncoder.inl",
			"%{prj.location}/../contrib/Open3DGC/o3dgcTimer.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcTools.cpp",
			"%{prj.location}/../contrib/Open3DGC/o3dgcTriangleFans.cpp",
			"%{prj.location}/../contrib/Open3DGC/o3dgcTriangleFans.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcTriangleListDecoder.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcTriangleListDecoder.inl",
			"%{prj.location}/../contrib/Open3DGC/o3dgcTriangleListEncoder.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcTriangleListEncoder.inl",
			"%{prj.location}/../contrib/Open3DGC/o3dgcVector.h",
			"%{prj.location}/../contrib/Open3DGC/o3dgcVector.inl",

			-- Loaders (Custom)
			"%{prj.location}/AssetLib/Obj/**",
			"%{prj.location}/AssetLib/Assbin/**"
		}


		-- Importer defines, assimp defines the importers NOT used,
		-- simply comment out the ones to use
		defines
		{
			'OPENDDLPARSER_BUILD',
			'OPENDDL_STATIC_LIBARY',
			'_SCL_SECURE_NO_WARNINGS',
			'_CRT_SECURE_NO_WARNINGS',
			'RAPIDJSON_HAS_STDSTRING=1',
			'RAPIDJSON_NOMEMBERITERATORCLASS',
			'ASSIMP_DOUBLE_PRECISION',
			'WIN32_LEAN_AND_MEAN',
			'UNICODE',
			'_UNICODE',
			-- 'ASSIMP_BUILD_NO_OWN_ZLIB',
			'ASSIMP_BUILD_BOOST_WORKAROUND',

			'ASSIMP_BUILD_NO_3D_IMPORTER',
			'ASSIMP_BUILD_NO_3DS_IMPORTER',
			'ASSIMP_BUILD_NO_3MF_IMPORTER',
			'ASSIMP_BUILD_NO_AC_IMPORTER',
			'ASSIMP_BUILD_NO_AMF_IMPORTER',
			'ASSIMP_BUILD_NO_ASE_IMPORTER',
			-- 'ASSIMP_BUILD_NO_ASSBIN_IMPORTER'
			'ASSIMP_BUILD_NO_B3D_IMPORTER',
			'ASSIMP_BUILD_NO_BLEND_IMPORTER',
			'ASSIMP_BUILD_NO_BVH_IMPORTER',
			'ASSIMP_BUILD_NO_C4D_IMPORTER',
			'ASSIMP_BUILD_NO_COB_IMPORTER',
			'ASSIMP_BUILD_NO_COLLADA_IMPORTER',
			'ASSIMP_BUILD_NO_CSM_IMPORTER',
			'ASSIMP_BUILD_NO_DXF_IMPORTER',
			'ASSIMP_BUILD_NO_FBX_IMPORTER',
			'ASSIMP_BUILD_NO_GLTF_IMPORTER',
			'ASSIMP_BUILD_NO_HMP_IMPORTER',
			'ASSIMP_BUILD_NO_IFC_IMPORTER',
			'ASSIMP_BUILD_NO_IRR_IMPORTER',
			'ASSIMP_BUILD_NO_IRRMESH_IMPORTER',
			'ASSIMP_BUILD_NO_LWO_IMPORTER',
			'ASSIMP_BUILD_NO_LWS_IMPORTER',
			'ASSIMP_BUILD_NO_M3D_IMPORTER',
			'ASSIMP_BUILD_NO_MD2_IMPORTER',
			'ASSIMP_BUILD_NO_MD3_IMPORTER',
			'ASSIMP_BUILD_NO_MD5_IMPORTER',
			'ASSIMP_BUILD_NO_MDC_IMPORTER',
			'ASSIMP_BUILD_NO_MDL_IMPORTER',
			'ASSIMP_BUILD_NO_MMD_IMPORTER',
			'ASSIMP_BUILD_NO_MS3D_IMPORTER',
			'ASSIMP_BUILD_NO_NDO_IMPORTER',
			'ASSIMP_BUILD_NO_NFF_IMPORTER',
			-- 'ASSIMP_BUILD_NO_OBJ_IMPORTER',
			'ASSIMP_BUILD_NO_OFF_IMPORTER',
			'ASSIMP_BUILD_NO_OGRE_IMPORTER',
			'ASSIMP_BUILD_NO_OPENGEX_IMPORTER',
			'ASSIMP_BUILD_NO_PLY_IMPORTER',
			'ASSIMP_BUILD_NO_Q3BSP_IMPORTER',
			'ASSIMP_BUILD_NO_Q3D_IMPORTER',
			'ASSIMP_BUILD_NO_RAW_IMPORTER',
			'ASSIMP_BUILD_NO_SIB_IMPORTER',
			'ASSIMP_BUILD_NO_SMD_IMPORTER',
			'ASSIMP_BUILD_NO_STEP_IMPORTER',
			'ASSIMP_BUILD_NO_STL_IMPORTER',
			'ASSIMP_BUILD_NO_TERRAGEN_IMPORTER',
			'ASSIMP_BUILD_NO_X_IMPORTER',
			'ASSIMP_BUILD_NO_X3D_IMPORTER',
			'ASSIMP_BUILD_NO_XGL_IMPORTER'
		}

		-- Exporters
		-- Same goes for exporters as importers
		defines
		{
			'ASSIMP_BUILD_NO_COLLADA_EXPORTER',
			'ASSIMP_BUILD_NO_X_EXPORTER',
			'ASSIMP_BUILD_NO_STEP_EXPORTER',
			'ASSIMP_BUILD_NO_OBJ_EXPORTER',
			'ASSIMP_BUILD_NO_STL_EXPORTER',
			'ASSIMP_BUILD_NO_PLY_EXPORTER',
			'ASSIMP_BUILD_NO_3DS_EXPORTER',
			'ASSIMP_BUILD_NO_GLTF_EXPORTER',
			'ASSIMP_BUILD_NO_ASSBIN_EXPORTER',
			'ASSIMP_BUILD_NO_ASSXML_EXPORTER',
			'ASSIMP_BUILD_NO_X3D_EXPORTER',
			'ASSIMP_BUILD_NO_FBX_EXPORTER',
			'ASSIMP_BUILD_NO_M3D_EXPORTER',
			'ASSIMP_BUILD_NO_3MF_EXPORTER',
			'ASSIMP_BUILD_NO_ASSJSON_EXPORTER'
		}

		filter "configurations:Debug"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			runtime "Release"
			optimize "on"
end

group "assimp"
	-- generate_zlib()
	-- generate_openddl_parser()
	generate_assimp()

	defines
	{
		"WIN32_LEAN_AND_MEAN",
		"ASSIMP_DOUBLE_PRECISION"
	}

	includedirs
	{
		project_path,
		project_path .. "/code",
	}

group ""