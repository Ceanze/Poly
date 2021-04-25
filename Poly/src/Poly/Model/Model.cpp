#include "polypch.h"
#include "Model.h"
#include "Mesh.h"

namespace Poly
{
	void Model::AddMeshInstance(MeshInstance meshInstance)
	{
		m_MeshInstances.push_back(meshInstance);
	}
	
	void Model::RemoveMeshInstance(MeshInstance meshInstance)
	{
		auto it = std::find_if(m_MeshInstances.begin(), m_MeshInstances.end(), [meshInstance](const MeshInstance& other){
			return other.MeshID == meshInstance.MeshID && other.ModelID ==  meshInstance.ModelID;
		});

		if (it == m_MeshInstances.end())
			POLY_CORE_WARN("Could not remove mesh instance with meshID {} and materialID {}, could not be found", meshInstance.MeshID, meshInstance.ModelID);

		m_MeshInstances.erase(it);
	}


}