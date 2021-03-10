#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	/**
	 * Resource is a RenderGraph specific object which abstracts the resources that it uses.
	 * The current implementation allows for Textures and Buffers to be used.
	 */

	class Texture;
	class Buffer;

	class Resource
	{
	public:
		Resource(Ref<Buffer> pBuffer, const std::string& name = "");
		Resource(Ref<Texture> pTexture, const std::string& name = "");
		~Resource() = default;

		/**
		 * @return True if resource is a buffer resource object
		 */
		bool IsBuffer() const { return m_pBuffer != nullptr; }

		/**
		 * @return True if resource is a texture resource object
		 */
		bool IsTexture() const { return m_pTexture != nullptr; }

		/**
		 * Get the resource as a buffer
		 * @return Resource as buffer, nullptr if not buffer
		 */
		Buffer* GetAsBuffer() const { return m_pBuffer.get(); }

		/**
		 * Get the resource as a texture
		 * @return Resource as texture, nullptr if not texture
		 */
		Texture* GetAsTexture() const { return m_pTexture.get(); }

		/**
		 * @return name of resource used in rendergraph
		 */
		const std::string& GetName() const { return m_Name; }

		/**
		 * @return current queue index
		 */
		uint32 GetCurrentQueueIndex() const { return m_CurrentQueueIndex; }

		/**
		 * @return current layout, UNDEFINED if resource is buffer
		 */
		ETextureLayout GetCurrentLayout() const { return m_CurrentLayout; }

		/**
		 * Create a resource based on a buffer
		 * NOTE: Must be ref to make sure resource is kept alive
		 * @param pBuffer - Ref to the buffer to be created as a resource
		 */
		static Ref<Resource> Create(Ref<Buffer> pBuffer, const std::string& name = "");

		/**
		 * Create a resource based on a texture
		 * NOTE: Must be ref to make sure resource is kept alive
		 * @param pTexture - Ref to the texture to be created as a resource
		 */
		static Ref<Resource> Create(Ref<Texture> pTexture, const std::string& name = "");

	private:
		friend class SyncPass;
		Ref<Buffer>		m_pBuffer	= nullptr;
		Ref<Texture>	m_pTexture	= nullptr;
		std::string		m_Name		= "";
		uint32			m_CurrentQueueIndex	= 0;
		ETextureLayout	m_CurrentLayout		= ETextureLayout::UNDEFINED;
	};
}