#pragma once

#include "Poly/Model/Model.h"

namespace Poly
{
	class Buffer;
	class RenderContext;
	class DescriptorSet;
	class PipelineLayout;

	struct DrawObject
	{
		MeshInstance			UniqueMeshInstance;
		std::vector<glm::mat4>	Matrices;
		Ref<Buffer>				pInstanceBuffer;
		Ref<DescriptorSet>		pVertexDescriptorSet;
		Ref<DescriptorSet>		pTextureDescriptorSet;
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
		uint32	GetMatrixCount(const std::unordered_map<size_t, DrawObject>& drawObjects);
		uint32	GetMatrixCount();

		std::vector<PolyID>						m_Models;
	};
}

// Step 1: Find all unique pairs of mesh and material (aka. unique mesh instance)
// Step 2: For each unique pair create a vector or similar to contain their respective data (transform matrices)
// Step 3: For all models save the matrix in the corresponding vector or create a new vector if a new unique pair (see step 2)
// Step 4: With all unique pairs of mesh and material - do an instanced draw with the corresponding matrices vector

// This means that the shader to be used needs to take in the following:
//	- Vertices (in form of Vertex buffer)
//	- Indices (in form of Index Buffer)
//	- Model Matrices (in form of uniform buffer)
//	- (Texture sampler)

// See TestPass.cpp::Execute on how the call to scene::draw could look like