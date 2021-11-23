#include "polypch.h"
#include "Scene.h"

#include "Poly/Model/Mesh.h"
#include "Poly/Core/RenderAPI.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/DescriptorSet.h"
#include "Platform/API/PipelineLayout.h"
#include "Poly/Resources/ResourceManager.h"
#include "Poly/Rendering/RenderGraph/RenderContext.h"

namespace Poly
{
	void Scene::AddModel(PolyID model)
	{
		m_Models.push_back(model);
	}

	void Scene::RemoveModel(PolyID model)
	{
		auto it = std::find(m_Models.begin(), m_Models.end(), model);
		if (it == m_Models.end())
			POLY_CORE_WARN("Cannot remove modelID {}, model could not be found", model);

		m_Models.erase(it);
	}



	// TODO: Move these functions to scenerenderer

	void Scene::OrderModels(std::unordered_map<size_t, DrawObject>& drawObjects)
	{
		for (auto modelID : m_Models)
		{
			Model* model = ResourceManager::GetModel(modelID);
			auto& meshInstances = model->GetMeshInstances();
			for (auto& meshInstance : meshInstances)
			{
				size_t hash = meshInstance.GetUniqueHash();

				if(!drawObjects.contains(hash))
				{
					drawObjects[hash].UniqueMeshInstance = meshInstance;
					drawObjects[hash].Matrices.push_back(model->GetTransform() * meshInstance.Transform);
					drawObjects[hash].pMaterial = ResourceManager::GetMaterial(meshInstance.MaterialID);
				}
				else
				{
					drawObjects[hash].Matrices.push_back(model->GetTransform() * meshInstance.Transform);
					drawObjects[hash].pMaterial = ResourceManager::GetMaterial(meshInstance.MaterialID);
				}
			}
		}
	}

	uint32 Scene::GetTotalMatrixCount(const std::unordered_map<size_t, DrawObject>& drawObjects)
	{
		uint32 count = 0;
		for (auto& drawObject : drawObjects)
			count += drawObject.second.Matrices.size();
		return count;
	}

	uint32 Scene::GetTotalMatrixCount()
	{
		uint32 count = 0;
		for (auto modelID : m_Models)
		{
			Model* model = ResourceManager::GetModel(modelID);
			auto& meshInstances = model->GetMeshInstances();
			count += meshInstances.size();
		}

		return count;
	}

}