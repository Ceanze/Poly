#include "ResourceUsage.h"

#include "Poly/RenderGraph/Feature/FeaturePort.h"

namespace Poly
{
	bool IsTextureResourceType(EResourceType type)
	{
		return type == EResourceType::SampledImage || type == EResourceType::StorageImage;
	}

	bool IsBufferResourceType(EResourceType type)
	{
		switch (type)
		{
		case EResourceType::UniformTexelBuffer:
		case EResourceType::StorageTexelBuffer:
		case EResourceType::StorageBuffer:
		case EResourceType::StorageBufferReadWrite:
		case EResourceType::RawBuffer:
		case EResourceType::RawBufferReadWrite:
		case EResourceType::UniformBuffer:
		case EResourceType::DynamicUniformBuffer:
			return true;
		default:
			return false;
		}
	}

	bool IsAttachmentSemanticPort(const std::string& resolvedName)
	{
		return resolvedName == ToSemanticName(EFeaturePort::Color) || resolvedName == ToSemanticName(EFeaturePort::Depth) ||
		       resolvedName == ToSemanticName(EFeaturePort::Stencil);
	}

	FPipelineStage DerivePassShaderStages(const std::vector<std::pair<std::string, FShaderStage>>& shaders)
	{
		FPipelineStage stages = FPipelineStage::NONE;
		for (const auto& [path, stage] : shaders)
		{
			if (BitsSet(stage, FShaderStage::VERTEX))
				stages |= FPipelineStage::VERTEX_SHADER;
			if (BitsSet(stage, FShaderStage::FRAGMENT))
				stages |= FPipelineStage::FRAGMENT_SHADER;
			if (BitsSet(stage, FShaderStage::COMPUTE))
				stages |= FPipelineStage::COMPUTE_SHADER;
		}

		return stages == FPipelineStage::NONE ? FPipelineStage::ALL_COMMANDS : stages;
	}

	ResourceUsage DeriveResourceUsage(EResourceType type, bool isWrite, const std::string& resolvedName,
	                                  FPipelineStage passShaderStages)
	{
		if (resolvedName == ToSemanticName(EFeaturePort::Color))
			return {ETextureLayout::COLOR_ATTACHMENT_OPTIMAL, FAccessFlag::COLOR_ATTACHMENT_READ | FAccessFlag::COLOR_ATTACHMENT_WRITE,
			        FPipelineStage::COLOR_ATTACHMENT_OUTPUT, FImageViewFlag::COLOR};

		if (resolvedName == ToSemanticName(EFeaturePort::Depth) || resolvedName == ToSemanticName(EFeaturePort::Stencil))
			return {ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			        FAccessFlag::DEPTH_STENCIL_ATTACHMENT_READ | FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE,
			        FPipelineStage::EARLY_FRAGMENT_TEST | FPipelineStage::LATE_FRAGMENT_TEST, FImageViewFlag::DEPTH_STENCIL};

		switch (type)
		{
		case EResourceType::SampledImage:
			return {ETextureLayout::SHADER_READ_ONLY_OPTIMAL, FAccessFlag::SHADER_READ, passShaderStages, FImageViewFlag::COLOR};

		case EResourceType::StorageImage:
			return {ETextureLayout::GENERAL, isWrite ? FAccessFlag::SHADER_WRITE : FAccessFlag::SHADER_READ, passShaderStages,
			        FImageViewFlag::COLOR};

		case EResourceType::UniformBuffer:
		case EResourceType::DynamicUniformBuffer:
			return {ETextureLayout::UNDEFINED, FAccessFlag::UNIFORM_READ, passShaderStages, FImageViewFlag::NONE};

		case EResourceType::StorageBuffer:
		case EResourceType::StorageBufferReadWrite:
		case EResourceType::RawBuffer:
		case EResourceType::RawBufferReadWrite:
		case EResourceType::UniformTexelBuffer:
		case EResourceType::StorageTexelBuffer:
			return {ETextureLayout::UNDEFINED, isWrite ? FAccessFlag::SHADER_WRITE : FAccessFlag::SHADER_READ, passShaderStages,
			        FImageViewFlag::NONE};

		// Not yet used by any pass in a way that requires barrier tracking - no sync.
		case EResourceType::Sampler:
		case EResourceType::PushConstants:
		case EResourceType::AccelerationStructure:
		case EResourceType::SamplerFeedback:
		case EResourceType::None:
		default:
			return {};
		}
	}

	ResourceUsage ConvertResourceState(FResourceState state)
	{
		switch (state)
		{
		case FResourceState::ShaderResource:
			return {ETextureLayout::SHADER_READ_ONLY_OPTIMAL, FAccessFlag::SHADER_READ, FPipelineStage::FRAGMENT_SHADER,
			        FImageViewFlag::COLOR};

		case FResourceState::UnorderedAccess:
			return {ETextureLayout::GENERAL, FAccessFlag::SHADER_READ | FAccessFlag::SHADER_WRITE, FPipelineStage::FRAGMENT_SHADER,
			        FImageViewFlag::COLOR};

		case FResourceState::RenderTarget:
			return {ETextureLayout::COLOR_ATTACHMENT_OPTIMAL, FAccessFlag::COLOR_ATTACHMENT_WRITE,
			        FPipelineStage::COLOR_ATTACHMENT_OUTPUT, FImageViewFlag::COLOR};

		case FResourceState::DepthWrite:
			return {ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL, FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE,
			        FPipelineStage::LATE_FRAGMENT_TEST, FImageViewFlag::DEPTH_STENCIL};

		case FResourceState::DepthRead:
			return {ETextureLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL, FAccessFlag::DEPTH_STENCIL_ATTACHMENT_READ,
			        FPipelineStage::EARLY_FRAGMENT_TEST, FImageViewFlag::DEPTH_STENCIL};

		case FResourceState::CopySource:
			return {ETextureLayout::TRANSFER_SRC_OPTIMAL, FAccessFlag::TRANSFER_READ, FPipelineStage::TRANSFER, FImageViewFlag::COLOR};

		case FResourceState::CopyDest:
			return {ETextureLayout::TRANSFER_DST_OPTIMAL, FAccessFlag::TRANSFER_WRITE, FPipelineStage::TRANSFER, FImageViewFlag::COLOR};

		case FResourceState::Present:
			return {ETextureLayout::PRESENT, FAccessFlag::MEMORY_READ, FPipelineStage::ALL_COMMANDS, FImageViewFlag::COLOR};

		case FResourceState::ConstantBuffer:
			return {ETextureLayout::UNDEFINED, FAccessFlag::UNIFORM_READ, FPipelineStage::ALL_COMMANDS, FImageViewFlag::NONE};

		default:
			POLY_CORE_WARN("Unhandled resource state ({}), falling back to a conservative barrier.", (int)state);
			return {ETextureLayout::GENERAL, FAccessFlag::MEMORY_READ | FAccessFlag::MEMORY_WRITE, FPipelineStage::ALL_COMMANDS,
			        FImageViewFlag::COLOR};
		}
	}
} // namespace Poly
