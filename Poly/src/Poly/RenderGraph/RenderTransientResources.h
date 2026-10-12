#pragma once

#include "ResourceManager.h"

#include <string>
#include <unordered_map>

namespace Poly
{
	/*
	 * Owns the textures a render program allocates on its own (depth, intermediate targets), on behalf of what
	 * they are rendered for: a view owns the ones of its view stage and the renderer the ones of the frame.
	 */
	class RenderTransientResources
	{
	public:
		RenderTransientResources() = default;
		~RenderTransientResources();
		CLASS_REMOVE_COPY(RenderTransientResources);

		/**
		 * Gets the texture with the given name, creating it first if it does not exist and recreating it if
		 * its size differs from the requested one.
		 * @param name - name of the resource, unique within this owner
		 * @param width - width of the texture
		 * @param height - height of the texture
		 * @param format - format the texture is created with
		 * @param usage - usage the texture is created with
		 * @param debugName - debug name the texture is created with
		 * @return handle of the texture
		 */
		TextureHandle GetOrCreate(const std::string& name, uint32 width, uint32 height, EFormat format, FTextureUsage usage, const std::string& debugName);

		/**
		 * Queues all textures for destruction
		 */
		void Clear();

	private:
		struct Entry
		{
			TextureHandle Handle;
			uint32        Width  = 0;
			uint32        Height = 0;
		};

		std::unordered_map<std::string, Entry> m_Entries;
	};
} // namespace Poly
