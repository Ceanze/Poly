#pragma once

#include "Platform/API/PipelineLayout.h"
#include "Platform/API/Sampler.h"
#include "Poly/Core/Core.h"

#include <map>
#include <mutex>

namespace Poly
{
	class Sampler;
	class TextureView;
	class DescriptorSet;
	class PipelineLayout;

	/*
	 * Owns the single global bindless descriptor heap used by every pass's pipeline layout (set 0),
	 * per plans/bindless.md: binding 0 = g_Textures[] (SAMPLED_IMAGE array, descriptor indexing),
	 * binding 1 = g_Samplers[] (SAMPLER array, descriptor indexing). Both are update-after-bind so
	 * new textures/samplers can be registered without waiting for in-flight frames to finish.
	 *
	 * Textures and samplers are registered into INDEPENDENT index spaces - MAX_TEXTURES (4096) is far
	 * bigger than MAX_SAMPLERS (256) because in practice thousands of textures share a small handful
	 * of sampler configs (filter/address-mode/aniso). Samplers are deduplicated by SamplerDesc value
	 * (s_SamplerCache), so registering the same config twice reuses the existing slot instead of
	 * burning through the 256-entry sampler heap. RegisterTextureAndSampler() packs both indices into
	 * one uint32 (see TEXTURE_INDEX_BITS/SAMPLER_INDEX_BITS) so a pass's textureIndices[] push-constant
	 * slot stays a single uint - the shader unpacks the two halves with a shift+mask.
	 *
	 * There is exactly one heap for the whole engine - it isn't per RenderProgramInstance. Every
	 * pass's own PipelineLayout still declares set 0 itself (via GetSetLayoutDesc(), copied in as
	 * that pass's set 0) so its VkPipelineLayout is structurally compatible with the heap's
	 * VkDescriptorSetLayout, but at draw time everyone binds the one shared DescriptorSet from
	 * GetDescriptorSet() - passes never create their own descriptor pool for set 0.
	 */
	class BindlessManager
	{
	public:
		CLASS_STATIC(BindlessManager);

		static constexpr uint32 MAX_TEXTURES = 4096;
		static constexpr uint32 MAX_SAMPLERS = 256;

		// Bit layout of the packed uint32 handed back by RegisterTextureAndSampler(): texture index in
		// the low TEXTURE_INDEX_BITS bits, sampler index directly above it. A shader unpacks with:
		//   uint textureIndex = packed & ((1u << TEXTURE_INDEX_BITS) - 1);
		//   uint samplerIndex = (packed >> TEXTURE_INDEX_BITS) & ((1u << SAMPLER_INDEX_BITS) - 1);
		static constexpr uint32 TEXTURE_INDEX_BITS  = 12; // log2(MAX_TEXTURES)
		static constexpr uint32 SAMPLER_INDEX_BITS  = 8;  // log2(MAX_SAMPLERS)
		static constexpr uint32 SAMPLER_INDEX_SHIFT = TEXTURE_INDEX_BITS;

		static_assert(MAX_TEXTURES <= (1u << TEXTURE_INDEX_BITS), "MAX_TEXTURES no longer fits TEXTURE_INDEX_BITS");
		static_assert(MAX_SAMPLERS <= (1u << SAMPLER_INDEX_BITS), "MAX_SAMPLERS no longer fits SAMPLER_INDEX_BITS");
		static_assert(TEXTURE_INDEX_BITS + SAMPLER_INDEX_BITS <= 32, "packed index no longer fits a uint32");

		static void Init();
		static void Release();

		// Registers pTextureView into g_Textures[] and pSampler (deduplicated by SamplerDesc value)
		// into g_Samplers[], and returns both indices packed into one uint32 for use in a pass's
		// resolved textureIndices[] slot - see TEXTURE_INDEX_BITS/SAMPLER_INDEX_BITS above for the bit
		// layout. The texture half is never deduplicated - callers that want to reuse a texture's index
		// should cache it themselves (e.g. keyed by resolved resource name, see
		// RenderProgramInstance::RuntimeResource); the sampler half is deduplicated automatically.
		static uint32 RegisterTextureAndSampler(const TextureView* pTextureView, ETextureLayout layout, Sampler* pSampler);

		// The set 0 layout description every pass must copy into its own PipelineLayoutDesc so its
		// pipeline layout stays structurally compatible with the heap's descriptor set.
		static const DescriptorSetLayout& GetSetLayoutDesc();

		// The one shared descriptor set to bind (set 0) before any pass using bindless resources runs.
		static DescriptorSet* GetDescriptorSet();

	private:
		// Orders SamplerDesc by value so it can key s_SamplerCache - samplers have no identity beyond
		// their config, so two Sampler objects with an identical desc must dedupe to the same slot.
		struct SamplerDescLess
		{
			bool operator()(const SamplerDesc& a, const SamplerDesc& b) const;
		};

		// Both assume the caller already holds s_Mutex.
		static uint32 RegisterTexture(const TextureView* pTextureView, ETextureLayout layout);
		static uint32 RegisterSampler(Sampler* pSampler);

		inline static Ref<PipelineLayout> s_pHeapPipelineLayout; // only used to own the set-0 VkDescriptorSetLayout
		inline static Ref<DescriptorSet>  s_pHeapSet;
		inline static DescriptorSetLayout s_SetLayoutDesc;

		// Guards s_NextTextureIndex/s_NextSamplerIndex/s_SamplerCache and the descriptor set writes in
		// RegisterTexture()/RegisterSampler() - both can be called concurrently from
		// RenderProgramInstance's multithreaded per-pass recording.
		inline static std::mutex s_Mutex;
		inline static uint32     s_NextTextureIndex = 0;
		inline static uint32     s_NextSamplerIndex = 0;

		// Sampler slot reuse, keyed by SamplerDesc value - keeps actual sampler heap usage far below
		// MAX_SAMPLERS even with thousands of textures registered, since most share a handful of configs.
		inline static std::map<SamplerDesc, uint32, SamplerDescLess> s_SamplerCache;
	};
} // namespace Poly
