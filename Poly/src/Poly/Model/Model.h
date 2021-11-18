#pragma once

#include <glm/glm.hpp>

namespace Poly
{
	class Mesh;
	struct MeshInstance;


	struct MeshInstance
	{
		MeshInstance() = default;
		MeshInstance(Ref<Mesh> pMesh, PolyID materialID, glm::mat4 transform) : pMesh(pMesh), MaterialID(materialID), Transform(transform) {}
		Ref<Mesh>	pMesh;
		PolyID		MaterialID;
		glm::mat4	Transform; // TEMP: Workaround until a ECS system which handles this seperate

		bool operator==(const MeshInstance& other) const { return pMesh == other.pMesh && MaterialID == other.MaterialID && Transform == other.Transform; }

		size_t GetUniqueHash() const { return std::hash<Ref<Mesh>>()(pMesh) ^ (std::hash<PolyID>()(MaterialID) >> 1); }
	};

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