#pragma once

/**
 * TODO:
 * RenderGraph:
 * 		- Vid AddExternalResource (med ResourceGroup och Resource) tillåt att sätta "autoBindDescriptor = true" för att manuellt kunna hantera
 * 		  bindning av descriptors. Detta är ett måste vid flera descriptors per resurs namn.
 *
 * RenderGraphProgram:
 * 		- UpdateGraphResource måste kunna hantera offset och index precis som descriptor cache, skapa overload av UpdateGraphResource(const std::string& name, uint64 size, const void* data)
 * 		  behöver nog inte en overload av den andra UpdateGraphResource varianten (den med Ref<Resource>)
 *
 * ResourceCahce:
 * 		- Tillåt att ha flera resurser per resursnamn (index biten i DescriptorCache och UpdateGraphResource)
 * 		- Synkroniseringen måste även hanteras i detta fall, rimligtvis genom att kolla om ett resursnamn har flera resurser bundet till sig
 * 		  och loopa igenom alla som den har. Borde med denna väg inte behöva göra om RenderGraphCompiler
 */

namespace Poly
{
	class DescriptorSet;
	class PipelineLayout;

	class DescriptorCache
	{
	private:
		struct CacheKey {
			uint32	Set			= 0;
			uint32	FrameIndex	= 0;

			CacheKey(uint32 frameIndex, uint32 set) : FrameIndex(frameIndex), Set(set) {}

			bool operator==(const CacheKey& other) const
			{
				return other.FrameIndex == FrameIndex && other.Set == Set;
			}
		};

		struct CacheKeyHasher
		{
			size_t operator()(const CacheKey& other) const
			{
				return (static_cast<uint64>(other.FrameIndex) << 32) | static_cast<uint64>(other.Set);
			}
		};

	public:
		DescriptorCache() = default;
		~DescriptorCache() = default;

		/**
		 * Sets the pipeline layout for this descriptor cache. Only one layout can be used per cache
		 * @param pPipelineLayout - the pipeline layout to use
		 */
		void SetPipelineLayout(PipelineLayout* pPipelineLayout);

		/**
		 * Gets/Creates a descriptor for the given set
		 * If an offset or index is necessary, use any of the overloaded functions
		 * @param set - The set the descriptor will be using
		 * @param frameIndex - The current frame index
		 * @return A cached or newly created descriptor set
		 */
		const DescriptorSet* GetDescriptorSet(uint32 set, uint32 frameIndex);

		/**
		 * Gets/Creates a descriptor for the given set
		 * If an offset in each buffer is necessary, use overloaded function
		 * If neither index nor offset is used, use the simple variant overloaded function
		 * @param set - The set the descriptor will be using
		 * @param frameIndex - The current frame index
		 * @param index - The index of the descriptor if multiple for the same set and frameindex combo. 0 if only one descriptor
		 * @return A cached or newly created descriptor set
		 */
		const DescriptorSet* GetDescriptorSet(uint32 set, uint32 frameIndex, uint32 index);

		/**
		 * Gets/Creates a descriptor for the given set
		 * If no offset is necessary, use overloaded function
		 * @param set - The set the descriptor will be using
		 * @param frameIndex - The current frame index
		 * @param index - The index of the descriptor if multiple for the same set and frameindex combo. 0 if only one descriptor
		 * @param offset - The offset for the descriptor in the current index, set, and frameindex combo. Offset is in bytes
		 * 					and is the same offset used when updating the descriptor (i.e. the offset in the buffer). 0 if only one descriptor
		 * @param segmentSize - Uniform size in bytes for each segment used for the offset bindings. 0 if only one descriptor
		 * @return A cached or newly created descriptor set
		 */
		const DescriptorSet* GetDescriptorSet(uint32 set, uint32 frameIndex, uint32 index, uint32 offset, uint32 segmentSize);

	private:
		Ref<DescriptorSet> CreateDescriptor(CacheKey key, uint32 index, uint32 offset);

		PipelineLayout* m_pPipelineLayout = nullptr;
		std::unordered_map<CacheKey, std::vector<std::vector<Ref<DescriptorSet>>>, CacheKeyHasher> m_Descriptors;
	};
}
