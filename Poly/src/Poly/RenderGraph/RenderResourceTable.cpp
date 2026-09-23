#include "RenderResourceTable.h"

namespace Poly
{
	void RenderResourceTable::Set(std::string_view name, BufferHandle handle)
	{
		m_Entries[std::string(name)] = {.BufHandle = handle};
	}

	void RenderResourceTable::Set(std::string_view name, TextureHandle handle, SamplerHandle sampler)
	{
		m_Entries[std::string(name)] = {.TexHandle = handle, .SamplerHnd = sampler};
	}

	void RenderResourceTable::Remove(std::string_view name)
	{
		m_Entries.erase(std::string(name));
	}
} // namespace Poly
