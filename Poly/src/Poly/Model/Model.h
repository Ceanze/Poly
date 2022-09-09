#pragma once

#include <glm/glm.hpp>

namespace Poly
{
	class Mesh;
	class Material;
	struct MeshInstance;

	struct MeshInstance
	{
		MeshInstance() = default;
		MeshInstance(Ref<Mesh> pMesh, Ref<Material> pMaterial) : pMesh(pMesh), pMaterial(pMaterial) {}
		Ref<Mesh>		pMesh;
		Ref<Material>	pMaterial;

		bool operator==(const MeshInstance& other) const { return pMesh == other.pMesh && pMaterial == other.pMaterial; }

		size_t GetUniqueHash() const { return std::hash<Ref<Mesh>>()(pMesh) ^ std::hash<Ref<Material>>()(pMaterial); }
	};

	class Model
	{
	public:
		Model() = default;
		~Model() = default;

		static Ref<Model> Create() { return CreateRef<Model>(); }

		void AddMeshInstance(MeshInstance meshInstance) { m_MeshInstances.push_back(meshInstance); }

		const std::vector<MeshInstance>& GetMeshInstances() const { return m_MeshInstances; }

		uint32 GetMeshInstanceCount() const { return static_cast<uint32>(m_MeshInstances.size()); }

		MeshInstance GetMeshInstance(uint32 meshIndex) const { return m_MeshInstances[meshIndex]; }

		Mesh* GetMesh(uint32 meshIndex) const { return m_MeshInstances[meshIndex].pMesh.get(); }

		Material* GetMaterial(uint32 meshIndex) const { return m_MeshInstances[meshIndex].pMaterial.get(); }

		PolyID GetPolyID() const { return m_ModelID; }

	private:
		friend class ResourceManager;
		void SetModelID(PolyID modelID) { m_ModelID = modelID; }

		std::vector<MeshInstance> m_MeshInstances;

		PolyID m_ModelID;
	};
}