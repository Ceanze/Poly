#pragma once

#include "Poly/Model/Model.h"

namespace Poly
{
	class Buffer;
	class Material;
	class RenderContext;
	class DescriptorSet;
	class PipelineLayout;

	struct DrawObject
	{
		MeshInstance			UniqueMeshInstance;
		std::vector<glm::mat4>	Matrices;
		Ref<DescriptorSet>		pInstanceDescriptorSet;
		Ref<DescriptorSet>		pVertexDescriptorSet;
		Ref<DescriptorSet>		pTextureDescriptorSet;
		Ref<DescriptorSet>		pMaterialDescriptorSet;
		Material*				pMaterial;
	};

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		void AddModel(PolyID model);

		void RemoveModel(PolyID model);

		const std::vector<PolyID>& GetModels() const { return m_Models; }

		uint32 GetModelCount() const { return m_Models.size(); }

		static Ref<Scene> Create() { return CreateRef<Scene>(); }

	private:
		using FramePassKey = std::pair<uint32, uint32>;
		friend class RenderGraphProgram;
		friend class SceneRenderer; // TEMP

		void	OrderModels(std::unordered_map<size_t, DrawObject>& drawObjects);
		uint32	GetTotalMatrixCount(const std::unordered_map<size_t, DrawObject>& drawObjects);
		uint32	GetTotalMatrixCount();

		std::vector<PolyID>						m_Models;
	};
}