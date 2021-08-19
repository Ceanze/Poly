#pragma once

#include "glm/glm.hpp"

namespace Poly
{
	class Buffer;
	class Material;

	struct Vertex
	{
		alignas(16)	glm::vec4	Position;
		alignas(16)	glm::vec4	Normal;
		alignas(16)	glm::vec4	Tangent;
		alignas(16)	glm::vec2	TexCoord;
	};

	class Mesh
	{
	public:
		Mesh() = default;
		~Mesh() = default;

		static Ref<Mesh> Create() { return CreateRef<Mesh>(); };

		void SetVertexBuffer(Ref<Buffer> pBuffer, uint32 vertexCount) { m_pVertexBuffer = pBuffer; m_VertexCount = vertexCount; }

		void SetIndexBuffer(Ref<Buffer> pBuffer, uint32 indexCount) { m_pIndexBuffer = pBuffer; m_IndexCount = indexCount; }

		const Buffer* GetVertexBuffer() const { return m_pVertexBuffer.get(); }

		const Buffer* GetIndexBuffer() const { return m_pIndexBuffer.get(); }

		uint32 GetVertexCount() const { return m_VertexCount; }

		uint32 GetIndexCount() const { return m_IndexCount; }

	private:
		Ref<Buffer> m_pVertexBuffer	= nullptr;
		Ref<Buffer> m_pIndexBuffer	= nullptr;
		uint32		m_VertexCount	= 0;
		uint32		m_IndexCount	= 0;
	};
}
