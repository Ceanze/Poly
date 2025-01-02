#pragma once

#include "Poly/Model/Model.h"

namespace Poly
{
	class Scene;
	class RenderGraphProgram;

	struct SceneBatch
	{
		MeshInstance	MeshInstance;
		uint32			InstanceCount;
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
		Scene& m_Scene;
		RenderGraphProgram& m_Program;

		std::unordered_map<size_t, int> m_InstanceHashToIndex;
		std::vector<SceneBatch> m_SceneBatches;
	};
}