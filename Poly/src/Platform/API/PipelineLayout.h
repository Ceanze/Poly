#pragma once

#include <vector>

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Sampler;

	struct PushConstantRange
	{
		FShaderStage	ShaderStage	= FShaderStage::NONE;
		uint32			Offset		= 0;
		uint32			Size		= 0;
	};

	struct DescriptorSetBinding
	{
		uint32			Binding			= 0;
		EDescriptorType	DescriptorType	= EDescriptorType::NONE;
		uint32			DescriptorCount	= 0;
		FShaderStage	ShaderStage		= FShaderStage::NONE;
		Sampler*		pSampler		= nullptr;
	};

	struct DescriptorSetLayout
	{
		std::vector<DescriptorSetBinding> DescriptorSetBindings;
	};

	struct PipelineLayoutDesc
	{
		std::vector<DescriptorSetLayout> DescriptorSetLayouts;
		std::vector<PushConstantRange> PushConstantRanges;
	};

	class PipelineLayout
	{
	public:
		CLASS_ABSTRACT(PipelineLayout);

		/**
		 * Init the Buffer object
		 * @param desc	Buffer creation description
		*/
		virtual void Init(const PipelineLayoutDesc* pDesc) = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

	protected:
		PipelineLayoutDesc p_PipelineLayoutDesc;
	};
}