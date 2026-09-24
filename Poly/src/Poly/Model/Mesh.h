#pragma once

#include "Platform/API/GraphicsPipeline.h"
#include "Poly/RenderGraph/BufferRange.h"
#include "Vertex.h"

#include <glm/glm.hpp>

namespace Poly
{
	class Model;
	class Buffer;
	class Material;

	// The alignas(16) on TexCoord forces trailing padding so this struct's size (and therefore its
	// per-element stride when indexed as an array via buffer_reference) is 64 bytes, matching
	// pbr_bindless.vert's BDA-side `vec4 TexCoord` - only .xy of that vec4 is ever read, so the extra
	// 8 bytes of padding this struct doesn't otherwise use are simply never touched by the shader.
	static_assert(sizeof(Vertex) == 64, "Vertex must keep a 64-byte stride to match pbr_bindless.vert's Vertex layout");

	struct MeshRange
	{
		BufferRange Vertices;
		BufferRange Indices;
	};

	class Mesh
	{
	public:
		Mesh(Model* pModel, MeshRange meshRange, uint32 meshIndex)
		    : m_pModel(pModel)
		    , m_MeshRange(std::move(meshRange))
		    , m_MeshIndex(meshIndex)
		{}
		~Mesh() = default;

		static Ref<Mesh> Create(Model* pModel, MeshRange meshRange, uint32 meshIndex) { return CreateRef<Mesh>(pModel, meshRange, meshIndex); }

		const MeshRange& GetMeshRange() const { return m_MeshRange; }

		uint32 GetMeshIndex() const { return m_MeshIndex; }

		Model* GetModel() const { return m_pModel; }

	private:
		MeshRange m_MeshRange;

		Model* m_pModel;
		uint32 m_MeshIndex;
	};
} // namespace Poly
