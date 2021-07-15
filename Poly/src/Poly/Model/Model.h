#pragma once

#include <glm/glm.hpp>

namespace Poly
{
	class Mesh;

	struct MeshInstance
	{
		MeshInstance() = default;
		MeshInstance(Ref<Mesh> pMesh, PolyID materialID) : pMesh(pMesh), MaterialID(materialID) {}
		Ref<Mesh>	pMesh;
		PolyID		MaterialID;

		bool operator==(const MeshInstance& other) const { return pMesh == other.pMesh && MaterialID == other.MaterialID; }
	};

	struct MeshInstanceKeyHasher
	{
		size_t operator()(const MeshInstance& key) const
		{
			return std::hash<Ref<Mesh>>()(key.pMesh) ^ (std::hash<PolyID>()(key.MaterialID) >> 1);
		}
	};

	class Model
	{
	public:
		Model() = default;
		~Model() = default;

		void AddMeshInstance(MeshInstance meshInstance) { m_Meshes.push_back(meshInstance); }

		const std::vector<MeshInstance> GetMeshInstances() const { return m_Meshes; }
		glm::mat4& GetTransform() const { return m_Transform; }

	private:
		std::vector<MeshInstance>	m_Meshes;
		glm::mat4					m_Transform;
	};
}