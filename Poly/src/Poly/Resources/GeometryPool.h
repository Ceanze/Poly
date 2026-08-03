#pragma once

#include "Poly/Model/Mesh.h"
#include "Poly/RenderGraph/BufferArena.h"

#include <vector>

namespace Poly
{
	struct MeshRange
	{
		BufferRange Vertices;
		BufferRange Indices;
	};

	/*
	 * Owns the engine-wide shared vertex and index buffers that all loaded meshes' geometry is
	 * appended into at load time, instead of each Mesh owning its own standalone buffer. Meshes
	 * are never unloaded today, so both underlying arenas only ever grow.
	 */
	class GeometryPool
	{
	public:
		CLASS_STATIC(GeometryPool);

		static void Init();
		static void Release();

		/*
		 * Appends a mesh's vertex/index data to the shared buffers, growing them if needed.
		 * @param vertices - CPU-side vertex data
		 * @param indices - CPU-side index data
		 * @return MeshRange - where the data landed in the shared vertex/index buffers
		 */
		static MeshRange UploadMesh(const std::vector<Vertex>& vertices, const std::vector<uint32>& indices);

		static BufferHandle GetVertexBufferHandle();
		static BufferHandle GetIndexBufferHandle();

	private:
		inline static BufferArena s_VertexArena{sizeof(Vertex), FBufferUsage::TRANSFER_SRC | FBufferUsage::STORAGE_BUFFER, EMemoryUsage::GPU_ONLY,
		                                        "GeometryPool.Vertices"};
		inline static BufferArena s_IndexArena{sizeof(uint32), FBufferUsage::TRANSFER_SRC | FBufferUsage::INDEX_BUFFER, EMemoryUsage::GPU_ONLY,
		                                       "GeometryPool.Indices"};
	};
} // namespace Poly
