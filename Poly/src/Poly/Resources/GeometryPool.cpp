#include "GeometryPool.h"

namespace Poly
{
	void GeometryPool::Init()
	{}

	void GeometryPool::Release()
	{
		s_VertexArena.Reset();
		s_IndexArena.Reset();
	}

	MeshRange GeometryPool::UploadMesh(const std::vector<Vertex>& vertices, const std::vector<uint32>& indices)
	{
		MeshRange range;
		range.Vertices = s_VertexArena.Upload(vertices.data(), static_cast<uint32>(vertices.size()));
		range.Indices  = s_IndexArena.Upload(indices.data(), static_cast<uint32>(indices.size()));
		return range;
	}

	BufferHandle GeometryPool::GetVertexBufferHandle()
	{
		return s_VertexArena.GetBufferHandle();
	}

	BufferHandle GeometryPool::GetIndexBufferHandle()
	{
		return s_IndexArena.GetBufferHandle();
	}
} // namespace Poly
