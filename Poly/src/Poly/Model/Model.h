#pragma once

#include <glm/glm.hpp>

namespace Poly
{
	class Mesh;
	struct MeshInstance;

	// namespace Internal { size_t MeshInstanceHasher(const MeshInstance& key); }

	struct MeshInstance
	{
		MeshInstance() = default;
		MeshInstance(Ref<Mesh> pMesh, PolyID materialID) : pMesh(pMesh), MaterialID(materialID) {}
		Ref<Mesh>	pMesh;
		PolyID		MaterialID;

		bool operator==(const MeshInstance& other) const { return pMesh == other.pMesh && MaterialID == other.MaterialID; }

		size_t GetUniqueHash() const { return std::hash<Ref<Mesh>>()(pMesh) ^ (std::hash<PolyID>()(MaterialID) >> 1); }
	};

	// struct MeshInstanceKeyHasher
	// {
	// 	size_t operator()(const MeshInstance& key) const
	// 	{
	// 		return Internal::MeshInstanceHasher(key);
	// 	}
	// };

	// namespace Internal
	// {
	// 	size_t MeshInstanceHasher(const MeshInstance& key)
	// 	{
	// 		return std::hash<Ref<Mesh>>()(key.pMesh) ^ (std::hash<PolyID>()(key.MaterialID) >> 1);
	// 	}
	// }

	class Model
	{
	public:
		Model() = default;
		~Model() = default;

		void AddMeshInstance(MeshInstance meshInstance) { m_Meshes.push_back(meshInstance); }

		const std::vector<MeshInstance>& GetMeshInstances() const { return m_Meshes; }
		const glm::mat4& GetTransform() const { return m_Transform; }

	private:
		std::vector<MeshInstance>	m_Meshes;
		glm::mat4					m_Transform;
	};
}