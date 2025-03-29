#pragma once

#include "Poly/Model/Model.h"

namespace Poly
{
	class Scene;
	class ResourceGUID;
	class RenderGraphProgram;

	struct SceneBatch
	{
		MeshInstance			MeshInstance;
		uint32					InstanceCount;
		std::vector<glm::mat4>	Matrices;
	};

	class RenderScene
	{
	public:
		RenderScene(Scene& scene, RenderGraphProgram& program);
		~RenderScene() = default;

		/**
		* Updates the Render Scene to generate new batches based on the linked scene
		*/
		void Update();

		/**
		* Get the current batches that was generated in the Update() call
		*/
		const std::vector<SceneBatch>& GetBatches() const;

	private:
		void CreateBufferIfNecessary(const ResourceGUID& bufferGUID, uint64 size);

		Scene& m_Scene;
		RenderGraphProgram& m_Program;

		uint32 m_TotalMeshCount;
		std::unordered_map<size_t, int> m_InstanceHashToIndex;
		std::vector<SceneBatch> m_SceneBatches;
	};
}