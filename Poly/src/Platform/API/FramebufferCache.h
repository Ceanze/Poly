#pragma once

namespace Poly
{
	class Framebuffer;
	class TextureView;
	class GraphicsRenderPass;

	class FramebufferCache
	{
	private:
	// Key used to access the unordered map with unique framebuffers dependent on attachments and render pass
	struct Key
	{
		/**
		 * Must be done when all of the members has been set
		 */
		void CreateHash();

		std::vector<TextureView*>	Attachments;
		TextureView*				pDepthAttachment	= nullptr;
		GraphicsRenderPass*			pPass				= nullptr;
		mutable size_t				Hash				= 0;

		bool operator== (const Key & other) const
		{
			return	pDepthAttachment == other.pDepthAttachment
					&& pPass == other.pPass
					&& Hash == other.Hash;
		}
	};

	struct KeyHasher
	{
		size_t operator()(const Key& key) const
		{
			return key.Hash;
		}
	};

	public:
		FramebufferCache() = default;
		~FramebufferCache() = default;

		/**
		 * Gets or creates a framebuffer from the cache
		 * @return framebuffer*
		 */
		Ref<Framebuffer> GetFramebuffer(const std::vector<TextureView*>& attachments, TextureView* pDepthAttachment, GraphicsRenderPass* pPass, uint32 width, uint32 height);

		/**
		 * Resets the cache - All framebuffers gotten from the cache will still be valid until they are no longer referenced
		 */
		void Reset() { m_Framebuffers.clear(); };

	private:
		std::unordered_map<Key, Ref<Framebuffer>, KeyHasher> m_Framebuffers;
	};
}