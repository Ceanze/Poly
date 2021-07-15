#include "polypch.h"
#include "Scene.h"

#include "Poly/Rendering/RenderGraph/RenderContext.h"
#include "Poly/Resources/ResourceManager.h"
#include "Poly/Model/Model.h"

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
}