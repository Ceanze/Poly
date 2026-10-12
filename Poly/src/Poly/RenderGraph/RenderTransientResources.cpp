#include "RenderTransientResources.h"

namespace Poly
{
	RenderTransientResources::~RenderTransientResources()
	{
		Clear();
	}

	TextureHandle RenderTransientResources::GetOrCreate(const std::string& name, uint32 width, uint32 height, EFormat format, FTextureUsage usage, const std::string& debugName)
	{
		Entry& entry = m_Entries[name];
		if (entry.Handle.IsValid() && entry.Width == width && entry.Height == height)
			return entry.Handle;

		// Destruction is deferred by the resource manager until no frame in flight uses the texture
		ResourceManager::Destroy(entry.Handle);

		entry.Handle = ResourceManager::CreateTexture2D(width, height, format, usage, debugName);
		entry.Width  = width;
		entry.Height = height;
		return entry.Handle;
	}

	void RenderTransientResources::Clear()
	{
		for (const auto& [name, entry] : m_Entries)
			ResourceManager::Destroy(entry.Handle);

		m_Entries.clear();
	}
} // namespace Poly
