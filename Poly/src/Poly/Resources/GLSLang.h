#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

// glslang
#include <StandAlone/DirStackFileIncluder.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/MachineIndependent/reflection.h>

#include <SPIRV/GlslangToSpv.h>

namespace Poly
{

	inline EShLanguage ConvertShaderStageGLSLang(FShaderStage shaderStage)
	{
		switch (shaderStage)
		{
		case FShaderStage::VERTEX:					return EShLangVertex;
		case FShaderStage::FRAGMENT:				return EShLangFragment;
		case FShaderStage::COMPUTE:					return EShLangCompute;
		case FShaderStage::TESSELLATION_CONTROL:	return EShLangTessControl;
		case FShaderStage::TESSELLATION_EVALUATION:	return EShLangTessEvaluation;
		case FShaderStage::GEOMETRY:				return EShLangGeometry;
		case FShaderStage::RAYGEN:					return EShLangRayGen;
		case FShaderStage::ANY_HIT:					return EShLangAnyHit;
		case FShaderStage::CLOSEST_HIT:				return EShLangClosestHit;
		case FShaderStage::MISS:					return EShLangMiss;
		case FShaderStage::INTERSECTION:			return EShLangIntersect;
		case FShaderStage::CALLABLE:				return EShLangCallable;
		case FShaderStage::TASK:					return EShLangTaskNV;
		case FShaderStage::MESH:					return EShLangMeshNV;
		default:									return EShLangCount;
		}
	}

	inline const TBuiltInResource* GetDefaultBuiltInResources()
	{
		static TBuiltInResource defaultBuiltInResources = {};

		defaultBuiltInResources.maxLights									= 32;
		defaultBuiltInResources.maxClipPlanes								= 6;
		defaultBuiltInResources.maxTextureUnits								= 32;
		defaultBuiltInResources.maxTextureCoords							= 32;
		defaultBuiltInResources.maxVertexAttribs							= 64;
		defaultBuiltInResources.maxVertexUniformComponents					= 4096;
		defaultBuiltInResources.maxVaryingFloats							= 64;
		defaultBuiltInResources.maxVertexTextureImageUnits					= 32;
		defaultBuiltInResources.maxCombinedTextureImageUnits				= 80;
		defaultBuiltInResources.maxTextureImageUnits						= 32;
		defaultBuiltInResources.maxFragmentUniformComponents				= 4096;
		defaultBuiltInResources.maxDrawBuffers								= 32;
		defaultBuiltInResources.maxVertexUniformVectors						= 128;
		defaultBuiltInResources.maxVaryingVectors							= 8;
		defaultBuiltInResources.maxFragmentUniformVectors					= 16;
		defaultBuiltInResources.maxVertexOutputVectors						= 16;
		defaultBuiltInResources.maxFragmentInputVectors						= 15;
		defaultBuiltInResources.minProgramTexelOffset						= -8;
		defaultBuiltInResources.maxProgramTexelOffset						= 7;
		defaultBuiltInResources.maxClipDistances							= 8;
		defaultBuiltInResources.maxComputeWorkGroupCountX					= 65535;
		defaultBuiltInResources.maxComputeWorkGroupCountY					= 65535;
		defaultBuiltInResources.maxComputeWorkGroupCountZ					= 65535;
		defaultBuiltInResources.maxComputeWorkGroupSizeX					= 1024;
		defaultBuiltInResources.maxComputeWorkGroupSizeY					= 1024;
		defaultBuiltInResources.maxComputeWorkGroupSizeZ					= 64;
		defaultBuiltInResources.maxComputeUniformComponents					= 1024;
		defaultBuiltInResources.maxComputeTextureImageUnits					= 16;
		defaultBuiltInResources.maxComputeImageUniforms						= 8;
		defaultBuiltInResources.maxComputeAtomicCounters					= 8;
		defaultBuiltInResources.maxComputeAtomicCounterBuffers				= 1;
		defaultBuiltInResources.maxVaryingComponents						= 60;
		defaultBuiltInResources.maxVertexOutputComponents					= 64;
		defaultBuiltInResources.maxGeometryInputComponents					= 64;
		defaultBuiltInResources.maxGeometryOutputComponents					= 128;
		defaultBuiltInResources.maxFragmentInputComponents					= 128;
		defaultBuiltInResources.maxImageUnits								= 8;
		defaultBuiltInResources.maxCombinedImageUnitsAndFragmentOutputs		= 8;
		defaultBuiltInResources.maxCombinedShaderOutputResources			= 8;
		defaultBuiltInResources.maxImageSamples								= 0;
		defaultBuiltInResources.maxVertexImageUniforms						= 0;
		defaultBuiltInResources.maxTessControlImageUniforms					= 0;
		defaultBuiltInResources.maxTessEvaluationImageUniforms				= 0;
		defaultBuiltInResources.maxGeometryImageUniforms					= 0;
		defaultBuiltInResources.maxFragmentImageUniforms					= 8;
		defaultBuiltInResources.maxCombinedImageUniforms					= 8;
		defaultBuiltInResources.maxGeometryTextureImageUnits				= 16;
		defaultBuiltInResources.maxGeometryOutputVertices					= 256;
		defaultBuiltInResources.maxGeometryTotalOutputComponents			= 1024;
		defaultBuiltInResources.maxGeometryUniformComponents				= 1024;
		defaultBuiltInResources.maxGeometryVaryingComponents				= 64;
		defaultBuiltInResources.maxTessControlInputComponents				= 128;
		defaultBuiltInResources.maxTessControlOutputComponents				= 128;
		defaultBuiltInResources.maxTessControlTextureImageUnits				= 16;
		defaultBuiltInResources.maxTessControlUniformComponents				= 1024;
		defaultBuiltInResources.maxTessControlTotalOutputComponents			= 4096;
		defaultBuiltInResources.maxTessEvaluationInputComponents			= 128;
		defaultBuiltInResources.maxTessEvaluationOutputComponents			= 128;
		defaultBuiltInResources.maxTessEvaluationTextureImageUnits			= 16;
		defaultBuiltInResources.maxTessEvaluationUniformComponents			= 1024;
		defaultBuiltInResources.maxTessPatchComponents						= 120;
		defaultBuiltInResources.maxPatchVertices							= 32;
		defaultBuiltInResources.maxTessGenLevel								= 64;
		defaultBuiltInResources.maxViewports								= 16;
		defaultBuiltInResources.maxVertexAtomicCounters						= 0;
		defaultBuiltInResources.maxTessControlAtomicCounters				= 0;
		defaultBuiltInResources.maxTessEvaluationAtomicCounters				= 0;
		defaultBuiltInResources.maxGeometryAtomicCounters					= 0;
		defaultBuiltInResources.maxFragmentAtomicCounters					= 8;
		defaultBuiltInResources.maxCombinedAtomicCounters					= 8;
		defaultBuiltInResources.maxAtomicCounterBindings					= 1;
		defaultBuiltInResources.maxVertexAtomicCounterBuffers				= 0;
		defaultBuiltInResources.maxTessControlAtomicCounterBuffers			= 0;
		defaultBuiltInResources.maxTessEvaluationAtomicCounterBuffers		= 0;
		defaultBuiltInResources.maxGeometryAtomicCounterBuffers				= 0;
		defaultBuiltInResources.maxFragmentAtomicCounterBuffers				= 1;
		defaultBuiltInResources.maxCombinedAtomicCounterBuffers				= 1;
		defaultBuiltInResources.maxAtomicCounterBufferSize					= 16384;
		defaultBuiltInResources.maxTransformFeedbackBuffers					= 4;
		defaultBuiltInResources.maxTransformFeedbackInterleavedComponents	= 64;
		defaultBuiltInResources.maxCullDistances							= 8;
		defaultBuiltInResources.maxCombinedClipAndCullDistances				= 8;
		defaultBuiltInResources.maxSamples									= 4;
		defaultBuiltInResources.limits.nonInductiveForLoops					= true;
		defaultBuiltInResources.limits.whileLoops							= true;
		defaultBuiltInResources.limits.doWhileLoops							= true;
		defaultBuiltInResources.limits.generalUniformIndexing				= true;
		defaultBuiltInResources.limits.generalAttributeMatrixVectorIndexing = true;
		defaultBuiltInResources.limits.generalVaryingIndexing				= true;
		defaultBuiltInResources.limits.generalSamplerIndexing				= true;
		defaultBuiltInResources.limits.generalVariableIndexing				= true;
		defaultBuiltInResources.limits.generalConstantMatrixVectorIndexing	= true;

		// Mesh shaders
		defaultBuiltInResources.maxMeshWorkGroupSizeX_NV	= 32;
		defaultBuiltInResources.maxMeshWorkGroupSizeY_NV	= 1;
		defaultBuiltInResources.maxMeshWorkGroupSizeZ_NV	= 1;
		defaultBuiltInResources.maxTaskWorkGroupSizeX_NV	= 32;
		defaultBuiltInResources.maxTaskWorkGroupSizeY_NV	= 1;
		defaultBuiltInResources.maxTaskWorkGroupSizeZ_NV	= 1;
		defaultBuiltInResources.maxMeshOutputVerticesNV		= 256;
		defaultBuiltInResources.maxMeshOutputPrimitivesNV	= 512;
		defaultBuiltInResources.maxMeshViewCountNV			= 4;

		return &defaultBuiltInResources;
	}

}
