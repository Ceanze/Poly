#include "Platform/API/Buffer.h"
#include "Platform/API/CommandBuffer.h"
#include "Platform/API/Sampler.h"
#include "Platform/API/Texture.h"
#include "Platform/API/TextureView.h"
#include "Poly.h"
#include "Poly/Core/Input/InputManager.h"
#include "Poly/Core/Logger.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Core/Window.h"
#include "Poly/Events/MouseEvent.h"
#include "Poly/Events/WindowEvent.h"
#include "Poly/RenderGraph/ExecuteContext.h"
#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/RenderGraph/RenderGraph.h"
#include "Poly/RenderGraph/RenderProgramInstance.h"
#include "Poly/RenderGraph/SceneRenderBridge.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/Resources/ResourceLoader.h"
#include "Poly/Resources/ResourceManager.h"
#include "Poly/Scene/Entity.h"
#include "Poly/Scene/Scene.h"

#include <imgui/imgui.h>

namespace
{
	struct CameraBuffer
	{
		glm::mat4 Mat;
		glm::vec4 Pos;
	};

	struct PointLight
	{
		glm::vec4 Color    = {1.0f, 1.0f, 1.0f, 1.0f};
		glm::vec4 Position = {0.0f, 1.0f, -1.0f, 1.0f};
	};

	struct LightBuffer
	{
		glm::vec4  LightCount = {1.0f, 0.0f, 0.0f, 0.0f};
		PointLight PointLight = {};
	};

	// Mirrors UIGlobalsBuffer in shaders/ui_bindless.vert byte-for-byte.
	struct UIGlobalsBuffer
	{
		glm::vec2 Scale;
		glm::vec2 Translate;
	};

	// Fixed-capacity ImGui vertex/index buffers, generous enough for ImGui::ShowDemoWindow(). Avoids
	// resizing them mid-session, which would need frame-in-flight-aware deferred destruction that
	// nothing else in RG2TestApp does yet (Camera/Lights follow the same single-buffer-updated-in-place
	// pattern).
	constexpr uint32 MAX_UI_VERTICES = 64 * 1024;
	constexpr uint32 MAX_UI_INDICES  = 128 * 1024;
} // namespace

class RG2TestLayer : public Poly::Layer
{
public:
	void OnAttach() override
	{
		Poly::Window* pWindow = Poly::Application::Get().GetWindow();

		m_pCamera = new Poly::Camera();
		m_pCamera->SetAspect(static_cast<float>(pWindow->GetWidth()) / pWindow->GetHeight());
		m_pCamera->SetMouseSense(2.f);
		m_pCamera->SetMovementSpeed(1.f);
		m_pCamera->SetSprintSpeed(5.f);

		m_pScene = Poly::Scene::Create("RG2TestScene");

		Poly::Entity cubeEntity = m_pScene->CreateEntity();
		// Poly::ResourceManager::ImportAndLoadModel("models/Cube/Cube.gltf", cubeEntity);
		Poly::ResourceManager::ImportAndLoadModel("models/sponza/gltf/sponza.gltf", cubeEntity);

		RegisterGeometryFeature();
		RegisterUIFeature();

		Poly::Ref<Poly::RenderProgram> pProgram = m_Graph.Begin()
		                                              .AddFeature("geometry")
		                                              .AddFeature("ui")
		                                              .WithFinalState(Poly::ToSemanticName(Poly::EFeaturePort::Color), Poly::FResourceState::Present)
		                                              .Build();

		Poly::Renderer* pRenderer = Poly::Application::Get().GetRenderer();
		pRenderer->SetScene(m_pScene);
		pRenderer->SetRenderProgram(pProgram);

		// TODO: Move to a different place (application wide resource handler?) - use a staging buffer
		Poly::BufferDesc cameraDesc = {};
		cameraDesc.Size             = sizeof(CameraBuffer);
		cameraDesc.MemUsage         = Poly::EMemoryUsage::CPU_VISIBLE;
		cameraDesc.BufferUsage      = Poly::FBufferUsage::UNIFORM_BUFFER | Poly::FBufferUsage::SHADER_DEVICE_ADDRESS;
		m_pCameraBuffer             = Poly::RenderAPI::CreateBuffer(&cameraDesc);

		Poly::BufferDesc lightsDesc = {};
		lightsDesc.Size             = sizeof(LightBuffer);
		lightsDesc.MemUsage         = Poly::EMemoryUsage::CPU_VISIBLE;
		lightsDesc.BufferUsage      = Poly::FBufferUsage::STORAGE_BUFFER | Poly::FBufferUsage::SHADER_DEVICE_ADDRESS;
		m_pLightsBuffer             = Poly::RenderAPI::CreateBuffer(&lightsDesc);

		LightBuffer lights = {};
		m_pLightsBuffer->TransferData(&lights, sizeof(LightBuffer), 0);

		// TODO: Temporary solution to update the camera and lights buffers in the render program instance
		Poly::RenderProgramInstance* pInstance = Poly::Application::Get().GetRenderer()->GetRenderProgramInstance();
		if (!pInstance)
			return;

		pInstance->UpdateResource("Camera", m_pCameraBuffer);
		pInstance->UpdateResource("Lights", m_pLightsBuffer);

		SetupUIResources(pInstance);
	}

	void OnUpdate(Poly::Timestamp dt) override
	{
		Poly::RenderProgramInstance* pInstance = Poly::Application::Get().GetRenderer()->GetRenderProgramInstance();
		if (!pInstance)
			return;

		if (!m_pScene->GetSceneRenderBridge())
		{
			// TODO: This is a temporary workaround. The scene render bridge (later RenderScene) should be created by the renderer pipeline automatically
			// however, currently it does not handle proper window management.
			Poly::Ref<Poly::RenderProgramInstance> nonOwningInstance(pInstance, [](Poly::RenderProgramInstance*) {});
			m_pScene->CreateSceneRenderBridge(nonOwningInstance);
		}

		m_pScene->Update();

		m_pCamera->Update(dt);
		CameraBuffer cameraData = {m_pCamera->GetMatrix(), m_pCamera->GetPosition()};
		m_pCameraBuffer->TransferData(&cameraData, sizeof(CameraBuffer), 0);

		UpdateUI();
	}

	void OnDetach() override { delete m_pCamera; }

	void OnEvent(Poly::Event& event) override
	{
		Poly::EventDispatcher eventDispatcher(event);
		eventDispatcher.Dispatch<Poly::Events::WindowResized>([this](Poly::Events::WindowResized& event) { return WindowResizeCallback(event); });
		eventDispatcher.Dispatch<Poly::Events::MouseButtonPressed>([this](Poly::Events::MouseButtonPressed& event) { return MouseButtonCallback(event.GetButton(), true); });
		eventDispatcher.Dispatch<Poly::Events::MouseButtonReleased>([this](Poly::Events::MouseButtonReleased& event) { return MouseButtonCallback(event.GetButton(), false); });
	}

private:
	// Current shader restriction means the order resources are registered must match the order they are bound in the shader. Until slang, this is the case
	// the order below is load bearing: Camera(0), scene.vertices(1), scene.instances(2), Lights(3), scene.materials(4).
	void RegisterGeometryFeature()
	{
		m_Graph.RegisterResource("Camera").WithType(Poly::EResourceType::UniformBuffer);
		m_Graph.RegisterResource("Lights").WithType(Poly::EResourceType::StorageBuffer);
		m_Graph.RegisterResource(Poly::Scene::VERTICES_RESOURCE_NAME_2).WithType(Poly::EResourceType::StorageBuffer);
		m_Graph.RegisterResource(Poly::Scene::INSTANCE_RESOURCE_NAME_2).WithType(Poly::EResourceType::StorageBuffer);
		m_Graph.RegisterResource(Poly::Scene::MATERIAL_RESOURCE_NAME_2).WithType(Poly::EResourceType::StorageBuffer);

		m_Graph.RegisterPass("pbr")
		    .WithShader("shaders/pbr_bindless.vert", Poly::FShaderStage::VERTEX)
		    .WithShader("shaders/pbr_bindless.frag", Poly::FShaderStage::FRAGMENT)
		    .MapResource(Poly::EFeaturePort::Color, "out_Color")
		    .MapResource(Poly::EFeaturePort::Depth, "depth")
		    .MapGlobal("Camera", "camera")
		    .MapGlobal(Poly::Scene::VERTICES_RESOURCE_NAME_2, "vertices")
		    .MapGlobal(Poly::Scene::INSTANCE_RESOURCE_NAME_2, "instances")
		    .MapGlobal("Lights", "lights")
		    .MapGlobal(Poly::Scene::MATERIAL_RESOURCE_NAME_2, "materialProps")
		    .WithGraphicsPipeline() // TODO: add a default pipeline to the graph so this can be omitted and the default used
		    .Topology(Poly::ETopology::TRIANGLE_LIST)
		    .PolygonMode(Poly::EPolygonMode::FILL)
		    .CullMode(Poly::ECullMode::BACK)
		    .ClockwiseFrontFace(false)
		    .DepthTestEnable(true)
		    .DepthWriteEnable(true)
		    .DepthCompareOp(Poly::ECompareOp::LESS_OR_EQUAL)
		    .AddColorBlendAttachment()
		    .BlendEnable(false)
		    .ColorWriteMask(Poly::FColorComponentFlag::RED | Poly::FColorComponentFlag::GREEN | Poly::FColorComponentFlag::BLUE |
		                    Poly::FColorComponentFlag::ALPHA)
		    .FinishColorBlendAttachment()
		    .FinishPipeline()
		    .WithExecuteFn([this](Poly::ExecuteContext& ctx) {
			    Poly::SceneRenderBridge* pBridge = m_pScene->GetSceneRenderBridge();
			    if (!pBridge)
				    return;

			    Poly::CommandBuffer* pCmd = ctx.GetCommandBuffer();
			    pCmd->BindIndexBuffer(pBridge->GetIndexBuffer(), 0, Poly::EIndexType::UINT32);
			    for (const Poly::SceneDrawBatch& batch : pBridge->GetDrawBatches())
				    pCmd->DrawIndexedInstanced(batch.IndexCount, batch.InstanceCount, batch.BaseIndex, batch.BaseVertex, batch.FirstInstance);
		    });

		m_Graph.RegisterFeature("geometry").WithPass("pbr");
	}

	// Font-only ImGui pass: only ever samples the font atlas, so its one texture resolves to a single
	// bindless slot built once per frame - no per-draw texture switching, which RG2 doesn't support yet
	// (ExecuteContext exposes no way to update push constants mid-pass; see ui_bindless.frag).
	void RegisterUIFeature()
	{
		m_Graph.RegisterResource("UIGlobals").WithType(Poly::EResourceType::UniformBuffer);
		m_Graph.RegisterResource("FontTexture").WithType(Poly::EResourceType::SampledImage);

		// clang-format off
		m_Graph.RegisterPass("ui")
		    .WithShader("shaders/ui_bindless.vert", Poly::FShaderStage::VERTEX)
		    .WithShader("shaders/ui_bindless.frag", Poly::FShaderStage::FRAGMENT)
		    .MapResource(Poly::EFeaturePort::Color, "out_Color") // auto-inferred LOAD op: "geometry" already wrote $Color first
		    .MapGlobal("UIGlobals", "globals")
		    .MapGlobal("FontTexture", "sTexture")
		    .WithGraphicsPipeline()
		    .AddVertexInput()
				.Binding(0)
				// TODO: Set Stride/VertexInputRate on the pipeline, not per-vertex-input, since PVKGraphicsPipeline only reads them off the first vertex input.
				// PVKGraphicsPipeline only reads Stride/VertexInputRate off VertexInputs[0] (one shared
				// binding for the whole pipeline) - has to be set here even though it describes binding 0
				// as a whole, not just the "pos" attribute.
				.Stride(sizeof(ImDrawVert))
				.VertexInputRate(Poly::EVertexInputRate::VERTEX)
				.Location(0)
				.Format(Poly::EFormat::R32G32_SFLOAT)
				.Offset(offsetof(ImDrawVert, pos))
		    .AddVertexInput()
				.Binding(0)
				.Location(1)
				.Format(Poly::EFormat::R32G32_SFLOAT)
				.Offset(offsetof(ImDrawVert, uv))
		    .AddVertexInput()
				.Binding(0)
				.Location(2)
				.Format(Poly::EFormat::R8G8B8A8_UNORM)
				.Offset(offsetof(ImDrawVert, col))
		    .FinishVertexInput()
		    .Topology(Poly::ETopology::TRIANGLE_LIST)
		    .PolygonMode(Poly::EPolygonMode::FILL)
		    .CullMode(Poly::ECullMode::NONE)
		    .ClockwiseFrontFace(true)
		    .ViewportDynamic(true)
		    .ScissorDynamic(true)
		    .DepthTestEnable(false)
		    .DepthWriteEnable(false)
		    .AddColorBlendAttachment()
				.BlendEnable(true)
				.SrcColorBlendFactor(Poly::EBlendFactor::SRC_ALPHA)
				.DstColorBlendFactor(Poly::EBlendFactor::ONE_MINUS_SRC_ALPHA)
				.ColorBlendOp(Poly::EBlendOp::ADD)
				.SrcAlphaBlendFactor(Poly::EBlendFactor::ONE_MINUS_SRC_ALPHA)
				.DstAlphaBlendFactor(Poly::EBlendFactor::ZERO)
				.AlphaBlendOp(Poly::EBlendOp::ADD)
				.ColorWriteMask(Poly::FColorComponentFlag::RED | Poly::FColorComponentFlag::GREEN | Poly::FColorComponentFlag::BLUE |
								Poly::FColorComponentFlag::ALPHA)
		    .FinishColorBlendAttachment()
		    .FinishPipeline()
		    .WithExecuteFn([this](Poly::ExecuteContext& ctx) {
			    ImDrawData* pDrawData = ImGui::GetDrawData();
			    if (!pDrawData || !pDrawData->Valid || pDrawData->CmdListsCount == 0)
				    return;

			    Poly::CommandBuffer* pCmd = ctx.GetCommandBuffer();
			    pCmd->BindVertexBuffer(m_pUIVertexBuffer.get(), 0, 1, 0);
			    pCmd->BindIndexBuffer(m_pUIIndexBuffer.get(), 0, Poly::EIndexType::UINT16);

			    uint32 vertexOffset = 0;
			    uint32 indexOffset  = 0;
			    for (int i = 0; i < pDrawData->CmdListsCount; i++)
			    {
				    const ImDrawList* pCmdList = pDrawData->CmdLists[i];
				    for (int j = 0; j < pCmdList->CmdBuffer.Size; j++)
				    {
					    const ImDrawCmd* pImCmd = &pCmdList->CmdBuffer[j];

					    Poly::ScissorDesc scissor = {};
					    scissor.OffsetX           = std::max(static_cast<int>(pImCmd->ClipRect.x), 0);
					    scissor.OffsetY           = std::max(static_cast<int>(pImCmd->ClipRect.y), 0);
					    scissor.Width             = static_cast<uint32>(pImCmd->ClipRect.z - pImCmd->ClipRect.x);
					    scissor.Height            = static_cast<uint32>(pImCmd->ClipRect.w - pImCmd->ClipRect.y);
					    pCmd->SetScissor(&scissor);

					    pCmd->DrawIndexedInstanced(pImCmd->ElemCount, 1, indexOffset, vertexOffset, 0);
					    indexOffset += pImCmd->ElemCount;
				    }
				    vertexOffset += pCmdList->VtxBuffer.Size;
			    }
		    });
		// clang-format on

		m_Graph.RegisterFeature("ui").WithPass("ui");
	}

	// Creates the font atlas texture/sampler and the fixed-capacity vertex/index/globals buffers, and
	// registers the font atlas + globals buffer with the render program instance once - only the
	// buffers' contents change per frame afterwards (see UpdateUI()), same pattern as Camera/Lights.
	void SetupUIResources(Poly::RenderProgramInstance* pInstance)
	{
		ImGuiIO& io = ImGui::GetIO();

		unsigned char* pFontData = nullptr;
		int            width = 0, height = 0;
		io.Fonts->GetTexDataAsRGBA32(&pFontData, &width, &height);

		m_pFontTexture = Poly::ResourceLoader::LoadTextureFromMemory(pFontData, width, height, 4, Poly::EFormat::R8G8B8A8_UNORM);

		Poly::TextureViewDesc viewDesc = {};
		viewDesc.pTexture              = m_pFontTexture.get();
		viewDesc.ImageViewType         = Poly::EImageViewType::TYPE_2D;
		viewDesc.ImageViewFlag         = Poly::FImageViewFlag::SHADER_RESOURCE;
		viewDesc.Format                = Poly::EFormat::R8G8B8A8_UNORM;
		viewDesc.MipLevelCount         = 1;
		viewDesc.ArrayLayerCount       = 1;
		m_pFontTextureView             = Poly::RenderAPI::CreateTextureView(&viewDesc);

		Poly::SamplerDesc samplerDesc = {};
		samplerDesc.MagFilter         = Poly::EFilter::LINEAR;
		samplerDesc.MinFilter         = Poly::EFilter::LINEAR;
		samplerDesc.MipMapMode        = Poly::ESamplerMipmapMode::LINEAR;
		samplerDesc.AddressModeU      = Poly::ESamplerAddressMode::CLAMP_TO_EDGE;
		samplerDesc.AddressModeV      = Poly::ESamplerAddressMode::CLAMP_TO_EDGE;
		samplerDesc.AddressModeW      = Poly::ESamplerAddressMode::CLAMP_TO_EDGE;
		samplerDesc.BorderColor       = Poly::EBorderColor::FLOAT_OPAQUE_WHITE;
		m_pFontSampler                = Poly::RenderAPI::CreateSampler(&samplerDesc);

		io.Fonts->TexID = (ImTextureID)m_pFontTextureView.get();

		Poly::BufferDesc globalsDesc = {};
		globalsDesc.Size             = sizeof(UIGlobalsBuffer);
		globalsDesc.MemUsage         = Poly::EMemoryUsage::CPU_VISIBLE;
		globalsDesc.BufferUsage      = Poly::FBufferUsage::UNIFORM_BUFFER | Poly::FBufferUsage::SHADER_DEVICE_ADDRESS;
		m_pUIGlobalsBuffer           = Poly::RenderAPI::CreateBuffer(&globalsDesc);

		Poly::BufferDesc vertexDesc = {};
		vertexDesc.Size             = MAX_UI_VERTICES * sizeof(ImDrawVert);
		vertexDesc.MemUsage         = Poly::EMemoryUsage::CPU_VISIBLE;
		vertexDesc.BufferUsage      = Poly::FBufferUsage::VERTEX_BUFFER;
		m_pUIVertexBuffer           = Poly::RenderAPI::CreateBuffer(&vertexDesc);

		Poly::BufferDesc indexDesc = {};
		indexDesc.Size             = MAX_UI_INDICES * sizeof(ImDrawIdx);
		indexDesc.MemUsage         = Poly::EMemoryUsage::CPU_VISIBLE;
		indexDesc.BufferUsage      = Poly::FBufferUsage::INDEX_BUFFER;
		m_pUIIndexBuffer           = Poly::RenderAPI::CreateBuffer(&indexDesc);

		pInstance->UpdateResource("FontTexture", m_pFontTextureView, m_pFontSampler);
		pInstance->UpdateResource("UIGlobals", m_pUIGlobalsBuffer);
	}

	// Builds this frame's ImGui draw data and uploads it - called once per frame from OnUpdate(),
	// before Renderer::Render() records the "ui" pass's command buffer.
	void UpdateUI()
	{
		ImGuiIO& io = ImGui::GetIO();

		// TODO: replace with real UI content; demo window only proves font-only text/widget rendering works.
		ImGui::ShowDemoWindow();
		ImGui::Render();

		ImDrawData* pDrawData = ImGui::GetDrawData();
		if (!pDrawData || !pDrawData->Valid)
			return;

		UIGlobalsBuffer globals = {};
		globals.Scale           = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		globals.Translate       = glm::vec2(-1.0f, -1.0f);
		m_pUIGlobalsBuffer->TransferData(&globals, sizeof(UIGlobalsBuffer), 0);

		uint64 vertexOffset = 0;
		uint64 indexOffset  = 0;
		for (int i = 0; i < pDrawData->CmdListsCount; i++)
		{
			const ImDrawList* pCmdList = pDrawData->CmdLists[i];

			uint64 vertexBufferSize = pCmdList->VtxBuffer.Size * sizeof(ImDrawVert);
			uint64 indexBufferSize  = pCmdList->IdxBuffer.Size * sizeof(ImDrawIdx);

			if (vertexOffset + vertexBufferSize > m_pUIVertexBuffer->GetSize() || indexOffset + indexBufferSize > m_pUIIndexBuffer->GetSize())
			{
				POLY_CORE_WARN("ImGui draw data exceeds RG2TestApp's fixed UI buffer capacity - dropping remaining draw lists");
				break;
			}

			m_pUIVertexBuffer->TransferData(pCmdList->VtxBuffer.Data, vertexBufferSize, vertexOffset);
			m_pUIIndexBuffer->TransferData(pCmdList->IdxBuffer.Data, indexBufferSize, indexOffset);

			vertexOffset += vertexBufferSize;
			indexOffset += indexBufferSize;
		}
	}

	bool WindowResizeCallback(Poly::Events::WindowResized& event)
	{
		m_pCamera->SetAspect(static_cast<float>(event.GetWidth()) / event.GetHeight());

		return true;
	}

	bool MouseButtonCallback(Poly::EKey button, bool pressed)
	{
		Poly::Window* pWindow = Poly::Application::Get().GetWindow();

		if (button == Poly::EKey::MOUSE_RIGHT && pressed)
			pWindow->SetMouseMode(Poly::EMouseMode::DISABLED);
		else if (button == Poly::EKey::MOUSE_RIGHT && !pressed)
			pWindow->SetMouseMode(Poly::EMouseMode::NORMAL);

		return true;
	}

	Poly::Camera*          m_pCamera = nullptr;
	Poly::Ref<Poly::Scene> m_pScene  = nullptr;
	Poly::RenderGraph      m_Graph;

	Poly::Ref<Poly::Buffer> m_pCameraBuffer;
	Poly::Ref<Poly::Buffer> m_pLightsBuffer;

	Poly::Ref<Poly::Texture>     m_pFontTexture;
	Poly::Ref<Poly::TextureView> m_pFontTextureView;
	Poly::Ref<Poly::Sampler>     m_pFontSampler;

	Poly::Ref<Poly::Buffer> m_pUIGlobalsBuffer;
	Poly::Ref<Poly::Buffer> m_pUIVertexBuffer;
	Poly::Ref<Poly::Buffer> m_pUIIndexBuffer;
};

class RG2TestApp : public Poly::Application
{
public:
	RG2TestApp() {}

	void OnInit() override { PushLayer(new RG2TestLayer()); }

private:
	std::optional<Poly::Window::Properties> GetWindowProperties() const override { return Poly::Window::Properties{1280, 720, "RG2 Test App"}; }
};

Poly::Application* Poly::CreateApplication()
{
	return new RG2TestApp();
}
