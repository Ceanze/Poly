/**
 * PolyRuntimeHost
 *
 * Standalone runtime process that renders a scene using the Poly engine and
 * exposes every frame via OS shared memory so that a separate editor process
 * (in any language/framework) can display the rendered viewport.
 *
 * Shared memory name : "PolyRuntimeFrame"  (see SharedFrameBuffer::DEFAULT_NAME)
 * Pixel format       : BGRA8 packed (matches swapchain default B8G8R8A8_UNORM)
 * Layout             : SharedFrameHeader (64 B) followed by two frame slots.
 *
 * Usage (editor side, pseudo-code):
 *   shm = OpenSharedMemory("PolyRuntimeFrame")
 *   last = 0
 *   loop:
 *     header = shm.ReadHeader()
 *     if header.frameCounter != last:
 *       pixels = shm.ReadSlot(header.activeSlot)
 *       UploadToGPUTexture(pixels, header.width, header.height)
 *       last = header.frameCounter
 *     DrawViewport()
 */

#include "Platform/API/Buffer.h"
#include "Poly.h"
#include "Poly/Core/Window.h"
#include "Poly/Events/WindowEvent.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/Rendering/RenderGraph/Passes/PBRPass.h"
#include "Poly/Rendering/RenderGraph/Passes/ReadTexturePass.h"
#include "Poly/Rendering/RenderGraph/RenderGraph.h"
#include "Poly/Rendering/RenderGraph/RenderGraphCompiler.h"
#include "Poly/Rendering/RenderGraph/RenderGraphProgram.h"
#include "Poly/Resources/ResourceManager.h"
#include "Poly/Scene/Entity.h"
#include "Poly/Scene/Scene.h"

class RuntimeHostLayer : public Poly::Layer
{
public:
	struct CameraBuffer
	{
		glm::mat4 Mat;
		glm::vec4 Pos;
	};

	struct PointLight
	{
		glm::vec4 Color    = {1.0f, 1.0f, 1.0f, 1.0f};
		glm::vec4 Position = {0.0f, 3.0f, 0.0f, 1.0f};
	};

	struct LightBuffer
	{
		glm::vec4  LightCount = {1.0f, 0.0f, 0.0f, 0.0f};
		PointLight PointLight = {};
	};

	RuntimeHostLayer() = default;

	void OnAttach() override
	{
		glm::vec2 windowSize = {1280.f, 720.f};

		m_pCamera = new Poly::Camera();
		m_pCamera->SetAspect(static_cast<float>(windowSize.x) / windowSize.y);
		m_pCamera->SetMouseSense(2.f);
		m_pCamera->SetMovementSpeed(1.f);
		m_pCamera->SetSprintSpeed(5.f);

		// Build render graph (PBR pass only — no ImGui overlay)
		m_pGraph                    = Poly::RenderGraph::Create("RuntimeHostGraph");
		Poly::Ref<Poly::Pass> pPass = Poly::PBRPass::Create();
		m_pScene                    = Poly::Scene::Create();

		m_pGraph->AddExternalResource(Poly::ResID("camera"), sizeof(CameraBuffer), Poly::FBufferUsage::UNIFORM_BUFFER);
		m_pGraph->AddExternalResource(Poly::ResID("lights"), sizeof(LightBuffer), Poly::FBufferUsage::STORAGE_BUFFER);
		m_pGraph->AddExternalResource(m_pScene->GetResourceGroup());

		m_pGraph->AddPass(pPass, Poly::PassID("pbrPass"));
		m_pGraph->AddLink(Poly::ResID("camera"), Poly::PassResID("pbrPass", "camera"));
		m_pGraph->AddLink(Poly::ResID("lights"), Poly::PassResID("pbrPass", "lights"));
		m_pGraph->AddLink(Poly::ResID("scene:albedoTex"), Poly::PassResID("pbrPass", "albedoTex"));
		m_pGraph->AddLink(Poly::ResID("scene:metallicTex"), Poly::PassResID("pbrPass", "metallicTex"));
		m_pGraph->AddLink(Poly::ResID("scene:normalTex"), Poly::PassResID("pbrPass", "normalTex"));
		m_pGraph->AddLink(Poly::ResID("scene:roughnessTex"), Poly::PassResID("pbrPass", "roughnessTex"));
		m_pGraph->AddLink(Poly::ResID("scene:aoTex"), Poly::PassResID("pbrPass", "aoTex"));
		m_pGraph->AddLink(Poly::ResID("scene:combinedTex"), Poly::PassResID("pbrPass", "combinedTex"));
		m_pGraph->AddLink(Poly::ResID("scene:vertices"), Poly::PassResID("pbrPass", "vertices"));
		m_pGraph->AddLink(Poly::ResID("scene:instance"), Poly::PassResID("pbrPass", "instances"));
		m_pGraph->AddLink(Poly::ResID("scene:material"), Poly::PassResID("pbrPass", "materialProps"));

		pReadTexturePass = Poly::ReadTexturePass::Create();
		m_pGraph->AddPass(pReadTexturePass, Poly::PassID("ReadTexturePass"));
		m_pGraph->AddLink(Poly::PassResID("pbrPass", "out_Color"), Poly::PassResID("ReadTexturePass", "InputTexture"));
		m_pGraph->AddMandatoryPass(Poly::PassID("ReadTexturePass"));

		m_pProgram = m_pGraph->Compile();

		LightBuffer lights = {};
		m_pProgram->UpdateGraphResource(Poly::ResID("lights").GetAsExternal(), sizeof(LightBuffer), &lights);
		m_pProgram->SetScene(m_pScene);

		Poly::Entity sceneEntity = m_pScene->CreateEntity();
		Poly::ResourceManager::ImportAndLoadModel("models/sponza/gltf/sponza.gltf", sceneEntity);

		// Set render graph and enable headless rendering
		Poly::Renderer* pRenderer = Poly::Application::Get().GetRenderer();
		pRenderer->EnableHeadless(windowSize.x, windowSize.y);
		pRenderer->SetRenderGraph(m_pProgram);
	}

	void OnUpdate(Poly::Timestamp dt) override
	{
		m_pScene->Update();
		m_pCamera->Update(dt);

		CameraBuffer camData = {m_pCamera->GetMatrix(), m_pCamera->GetPosition()};
		m_pProgram->UpdateGraphResource(Poly::ResID("camera").GetAsExternal(), sizeof(CameraBuffer), &camData);
	}

	void OnDetach() override
	{
		Poly::Application::Get().GetRenderer()->DisableHeadless();
		delete m_pCamera;
	}

	void OnEvent(Poly::Event& event) override
	{
		Poly::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Poly::Events::WindowResized>([this](Poly::Events::WindowResized& e) {
			m_pCamera->SetAspect(static_cast<float>(e.GetWidth()) / e.GetHeight());
			return true;
		});
	}

private:
	Poly::Camera*                       m_pCamera        = nullptr;
	Poly::Ref<Poly::Scene>              m_pScene         = nullptr;
	Poly::Ref<Poly::RenderGraph>        m_pGraph         = nullptr;
	Poly::Ref<Poly::RenderGraphProgram> m_pProgram       = nullptr;
	Poly::Ref<Poly::ReadTexturePass>    pReadTexturePass = nullptr;
};

class PolyRuntimeHost : public Poly::Application
{
public:
	PolyRuntimeHost() = default;

	void OnInit() override
	{
		PushLayer(new RuntimeHostLayer());
	}

private:
	std::optional<Poly::Window::Properties> GetWindowProperties() const override
	{
		// The runtime host uses a small window for the Vulkan surface.
		// The editor displays the rendered output via shared memory; the host
		// window can be kept visible for debugging or hidden as needed.
		// return Poly::Window::Properties{1280, 720, "Poly Runtime Host"};
		return std::nullopt;
	}
};

Poly::Application* Poly::CreateApplication()
{
	return new PolyRuntimeHost();
}
