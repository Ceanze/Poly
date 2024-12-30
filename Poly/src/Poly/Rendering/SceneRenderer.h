#pragma once

#include "Poly/Scene/Scene.h"
#include "Poly/Model/Model.h"
#include "Poly/Model/Material.h"

#include "Poly/Rendering/RenderGraph/PassReflection.h"

namespace Poly
{
	struct SceneBinding;

	class Scene;
	class Buffer;
	class RenderContext;
	class CommandBuffer;
	class DescriptorSet;
	class PipelineLayout;
	class PassReflection;

	struct DrawObject
	{
		MeshInstance			UniqueMeshInstance;
		std::vector<glm::mat4>	Matrices;
		DescriptorSet*			pInstanceDescriptorSet;
		DescriptorSet*			pVertexDescriptorSet;
		DescriptorSet*			pTextureDescriptorSet;
		DescriptorSet*			pMaterialDescriptorSet;
	};

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;
		~SceneRenderer() = default;

		void Execute(const RenderContext& context, const Scene& scene, const PassReflection& reflection, uint32 imageIndex);

	private:
		void Update(const RenderContext& context, const Scene& scene, const PassReflection& reflection, uint32 imageIndex);
		void Render(const RenderContext& context);

		void UpdateTextureDescriptor(const RenderContext& context, const PassReflection& reflection, DrawObject& drawObject, uint32 drawObjectIndex, uint32 imageIndex, ESceneBinding sceneBinding, Material::Type type);
		void UpdateInstanceBuffers(uint64 size);
		void UpdateMaterialBuffers(uint64 size);
		void OrderModels(const Scene& scene);

		Ref<Buffer> m_pInstanceBuffer;
		Ref<Buffer> m_pStagingBuffer;
		Ref<Buffer> m_pMaterialBuffer;
		Ref<Buffer> m_pMaterialStagingBuffer;

		std::unordered_map<size_t, DrawObject> m_DrawObjects;
		uint32 m_TotalMeshCount = 0;
	};

}