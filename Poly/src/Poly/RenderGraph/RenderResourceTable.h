#pragma once

#include "Poly/RenderGraph/ResourceManager.h"

#include <string>
#include <string_view>
#include <unordered_map>

namespace Poly
{
	/*
	 * Resources provided by a world to any render program instance rendering it.
	 */
	class RenderResourceTable
	{
	public:
		struct Entry
		{
			BufferHandle  BufHandle;
			TextureHandle TexHandle;
			SamplerHandle SamplerHnd;
		};

		/**
		 * Provides (or replaces) a buffer for the resource with the given name
		 * @param name - resolved name of the resource, as registered in the render catalog
		 * @param handle - buffer to provide
		 */
		void Set(std::string_view name, BufferHandle handle);

		/**
		 * Provides (or replaces) a texture for the resource with the given name
		 * @param name - resolved name of the resource, as registered in the render catalog
		 * @param handle - texture to provide
		 * @param sampler - sampler to use, invalid for the default linear sampler
		 */
		void Set(std::string_view name, TextureHandle handle, SamplerHandle sampler = {});

		/**
		 * Stops providing the resource with the given name
		 * @param name - resolved name of the resource
		 */
		void Remove(std::string_view name);

		const std::unordered_map<std::string, Entry>& GetEntries() const { return m_Entries; }

	private:
		std::unordered_map<std::string, Entry> m_Entries;
	};
} // namespace Poly
