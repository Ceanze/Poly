#include "BindlessManager.h"

#include "Platform/API/DescriptorSet.h"
#include "Poly/Core/RenderAPI.h"

#include <tuple>

namespace Poly
{
	void BindlessManager::Init()
	{
		// Note: Vulkan only allows VARIABLE_DESCRIPTOR_COUNT on the single highest-numbered binding in
		// a set, and this set has two array bindings - so both are declared with a fixed
		// MAX_TEXTURES/MAX_SAMPLERS count at layout-creation time instead (PARTIALLY_BOUND still lets
		// unused slots stay unwritten, UPDATE_AFTER_BIND still allows registering more while in flight).
		DescriptorSetBinding texturesBinding = {};
		texturesBinding.Binding              = 0;
		texturesBinding.DescriptorType       = EDescriptorType::SAMPLED_IMAGE;
		texturesBinding.DescriptorCount      = MAX_TEXTURES;
		texturesBinding.ShaderStage          = FShaderStage::VERTEX | FShaderStage::FRAGMENT;
		texturesBinding.BindingFlags         = FDescriptorBindingFlag::PARTIALLY_BOUND | FDescriptorBindingFlag::UPDATE_AFTER_BIND;

		DescriptorSetBinding samplersBinding = {};
		samplersBinding.Binding              = 1;
		samplersBinding.DescriptorType       = EDescriptorType::SAMPLER;
		samplersBinding.DescriptorCount      = MAX_SAMPLERS;
		samplersBinding.ShaderStage          = FShaderStage::VERTEX | FShaderStage::FRAGMENT;
		samplersBinding.BindingFlags         = FDescriptorBindingFlag::PARTIALLY_BOUND | FDescriptorBindingFlag::UPDATE_AFTER_BIND;

		s_SetLayoutDesc.DescriptorSetBindings = {texturesBinding, samplersBinding};

		PipelineLayoutDesc layoutDesc   = {};
		layoutDesc.DescriptorSetLayouts = {s_SetLayoutDesc};
		s_pHeapPipelineLayout           = RenderAPI::CreatePipelineLayout(&layoutDesc);

		s_pHeapSet = RenderAPI::CreateDescriptorSet(s_pHeapPipelineLayout.get(), 0);
	}

	void BindlessManager::Release()
	{
		s_pHeapSet.reset();
		s_pHeapPipelineLayout.reset();
		s_NextTextureIndex = 0;
		s_NextSamplerIndex = 0;
		s_SamplerCache.clear();
	}

	bool BindlessManager::SamplerDescLess::operator()(const SamplerDesc& a, const SamplerDesc& b) const
	{
		const auto AsTuple = [](const SamplerDesc& d)
		{
			return std::tie(d.MinFilter, d.MagFilter, d.AddressModeU, d.AddressModeV, d.AddressModeW, d.MipMapMode,
			                 d.BorderColor, d.MipLodBias, d.MinLod, d.MaxLod, d.AnistropyEnable, d.MaxAnisotropy);
		};
		return AsTuple(a) < AsTuple(b);
	}

	uint32 BindlessManager::RegisterTexture(const TextureView* pTextureView, ETextureLayout layout)
	{
		POLY_VALIDATE(s_NextTextureIndex < MAX_TEXTURES, "Bindless texture heap is full ({} textures)", MAX_TEXTURES);

		const uint32 index = s_NextTextureIndex++;
		s_pHeapSet->UpdateTextureBinding(0, layout, pTextureView, nullptr, index);
		return index;
	}

	uint32 BindlessManager::RegisterSampler(Sampler* pSampler)
	{
		auto it = s_SamplerCache.find(pSampler->GetDesc());
		if (it != s_SamplerCache.end())
			return it->second;

		POLY_VALIDATE(s_NextSamplerIndex < MAX_SAMPLERS, "Bindless sampler heap is full ({} samplers)", MAX_SAMPLERS);

		const uint32 index = s_NextSamplerIndex++;
		s_pHeapSet->UpdateSamplerBinding(1, pSampler, index);
		s_SamplerCache.emplace(pSampler->GetDesc(), index);
		return index;
	}

	uint32 BindlessManager::RegisterTextureAndSampler(const TextureView* pTextureView, ETextureLayout layout, Sampler* pSampler)
	{
		std::lock_guard<std::mutex> lock(s_Mutex);

		const uint32 textureIndex = RegisterTexture(pTextureView, layout);
		const uint32 samplerIndex = RegisterSampler(pSampler);

		return textureIndex | (samplerIndex << SAMPLER_INDEX_SHIFT);
	}

	const DescriptorSetLayout& BindlessManager::GetSetLayoutDesc()
	{
		return s_SetLayoutDesc;
	}

	DescriptorSet* BindlessManager::GetDescriptorSet()
	{
		return s_pHeapSet.get();
	}
} // namespace Poly
