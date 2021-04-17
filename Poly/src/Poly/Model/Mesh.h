#pragma once

#include "glm/glm.hpp"

namespace Poly
{
	class Buffer;
	class Material;

	struct Vertex
	{
		alignas(16)	glm::vec4	Position;
		alignas(16)	glm::vec2	TexCoord;
	};

	class Mesh
	{
	public:
		Mesh() = default;
		~Mesh() = default;

		static Ref<Mesh> Create() { return CreateRef<Mesh>(); };

		void SetVertexBuffer(Ref<Buffer> pBuffer) { m_pVertexBuffer = pBuffer; }

		void SetIndexBuffer(Ref<Buffer> pBuffer) { m_pIndexBuffer = pBuffer; }

		const Buffer* GetVertexBuffer() const { return m_pVertexBuffer.get(); }

		const Buffer* GetIndexBuffer() const { return m_pIndexBuffer.get(); }

	private:
		Ref<Buffer> m_pVertexBuffer	= nullptr;
		Ref<Buffer> m_pIndexBuffer	= nullptr;
	};

	struct MeshInstance
	{
		PolyID	pMesh		= 0;
		PolyID	pMaterial	= 0;
	};
}
