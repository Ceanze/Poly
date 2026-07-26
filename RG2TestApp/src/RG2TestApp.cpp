#include "Platform/API/Buffer.h"
#include "Platform/API/CommandBuffer.h"
#include "Poly.h"
#include "Poly/Core/Input/InputManager.h"
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
#include "Poly/Resources/ResourceManager.h"
#include "Poly/Scene/Entity.h"
#include "Poly/Scene/Scene.h"

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

		Poly::Ref<Poly::RenderProgram> pProgram = m_Graph.Begin()
		                                              .AddFeature("geometry")
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
