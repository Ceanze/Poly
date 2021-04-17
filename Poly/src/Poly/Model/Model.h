#pragma once

namespace Poly
{
	struct MeshInstance;

	class Model
	{
	public:
		Model() = default;
		~Model() = default;

		void AddMeshInstance(MeshInstance meshInstance);

		void RemoveMeshInstance(MeshInstance meshInstance);

		const std::vector<MeshInstance>& GetMeshInstances() const { return m_MeshInstances; }

		uint32 GetMeshInstanceCount() const { return m_MeshInstances.size(); }

	private:
		std::vector<MeshInstance> m_MeshInstances;
	};
}