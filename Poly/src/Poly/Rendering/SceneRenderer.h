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

		void SetScene(Ref<Scene> pScene) { m_pScene = pScene; }

		void Update(const RenderContext& context, const PassReflection& reflection, uint32 imageIndex, PipelineLayout* pPipelineLayout);

		void Render(const RenderContext& context);

		static Ref<SceneRenderer> Create() { return CreateRef<SceneRenderer>(); }

	private:
		void UpdateTextureDescriptor(const RenderContext& context, const PassReflection& reflection, DrawObject& drawObject, uint32 drawObjectIndex, uint32 imageIndex, ESceneBinding sceneBinding, Material::Type type);
		void UpdateInstanceBuffers(uint64 size);
		void UpdateMaterialBuffers(uint64 size);
		void OrderModels();

		Ref<Scene> m_pScene;

		Ref<Buffer> m_pInstanceBuffer;
		Ref<Buffer> m_pStagingBuffer;
		Ref<Buffer> m_pMaterialBuffer;
		Ref<Buffer> m_pMaterialStagingBuffer;

		std::unordered_map<size_t, DrawObject> m_DrawObjects;
		uint32 m_TotalMeshCount = 0;
	};

}