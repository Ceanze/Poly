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
		Ref<DescriptorSet>		pDescriptorSet;
	};


	struct FramePassHasher
	{
		size_t operator()(const std::pair<uint32, uint32>& other) const
		{
			return (static_cast<uint64>(other.first) << 32) | static_cast<uint64>(other.second);
		}
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

		/**
		 * Update updates the resources (descriptorSets) that Scene is using
		 * and must be called before Execute
		 */
		void Update(const RenderContext& context);

		void Execute(const RenderContext& context);

		const Buffer* const GetInstanceBuffer() const { return m_pInstanceBuffer.get(); }

		static Ref<Scene> Create() { return CreateRef<Scene>(); }

	private:
		using FramePassKey = std::pair<uint32, uint32>;
		friend class RenderGraphProgram;
		friend class SceneRenderer; // TEMP

		void	SetFrameIndex(uint32 frameIndex) { m_FrameIndex = frameIndex; }
		void	OrderModels(std::unordered_map<size_t, DrawObject>& drawObjects);
		Buffer* GetStagingBuffer(uint32 matrixCount, uint32 passIndex);
		void	HandleInstanceBufferSize(uint32 matrixCount, uint32 passIndex);
		uint32	GetMatrixCount(const std::unordered_map<size_t, DrawObject>& drawObjects);
		uint32	GetMatrixCount();
		Ref<DescriptorSet> GetDescriptorSet(const RenderContext& context);

		uint32									m_InstanceSet		= 0;
		PipelineLayout*							m_pPipelineLayout	= nullptr;
		std::vector<PolyID>						m_Models;
		Ref<Buffer>								m_pInstanceBuffer	= nullptr;
		std::unordered_map<uint32, Ref<Buffer>>	m_StagingBuffers;
		uint32									m_FrameIndex		= 0;
		std::unordered_map<FramePassKey, std::vector<Ref<Buffer>>, FramePassHasher> m_BuffersToBeDestroyed;
		std::unordered_map<uint32, Ref<DescriptorSet>> m_PassDescriptorSets;
		std::unordered_map<FramePassKey, std::vector<Ref<DescriptorSet>>, FramePassHasher> m_DescriptorSetsToBeDestroyed;
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