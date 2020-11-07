#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

// glslang
#include <StandAlone/DirStackFileIncluder.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/MachineIndependent/reflection.h>

namespace Poly
{

	EShLanguage ConvertShaderStageGLSLang(ShaderStage shaderStage)
	{
		switch (shaderStage)
		{
		case ShaderStage::VERTEX:					return EShLangVertex;
		case ShaderStage::FRAGMENT:					return EShLangFragment;
		case ShaderStage::COMPUTE:					return EShLangCompute;
		case ShaderStage::TESSELLATION_CONTROL:		return EShLangTessControl;
		case ShaderStage::TESSELLATION_EVALUATION:	return EShLangTessEvaluation;
		case ShaderStage::GEOMETRY:					return EShLangGeometry;
		case ShaderStage::RAYGEN:					return EShLangRayGen;
		case ShaderStage::ANY_HIT:					return EShLangAnyHit;
		case ShaderStage::CLOSEST_HIT:				return EShLangClosestHit;
		case ShaderStage::MISS:						return EShLangMiss;
		case ShaderStage::INTERSECTION:				return EShLangIntersect;
		case ShaderStage::CALLABLE:					return EShLangCallable;
		case ShaderStage::TASK:						return EShLangTaskNV;
		case ShaderStage::MESH:						return EShLangMeshNV;
		default:									return EShLangCount;
		}
	}

}