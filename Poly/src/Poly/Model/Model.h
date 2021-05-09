#pragma once

namespace Poly
{
	class Mesh;

	struct MeshInstance
	{
		MeshInstance() = default;
		MeshInstance(Ref<Mesh> pMesh, PolyID materialID) : pMesh(pMesh), MaterialID(materialID) {}
		Ref<Mesh>	pMesh;
		PolyID		MaterialID;
	};

	class Model
	{
	public:
		Model() = default;
		~Model() = default;

		void AddMeshInstance(MeshInstance meshInstance) { m_Meshes.push_back(meshInstance); }

		const std::vector<MeshInstance> GetMeshInstances() const { return m_Meshes; }

	private:
		std::vector<MeshInstance>	m_Meshes;
	};
}