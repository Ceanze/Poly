#include "Poly.h"

/*
Steg f�r skapning av ett enkelt PVK program:
	- Application kallar p� Renderer::preInit() som skapar Window, Instance, SwapChain, syncobjects
	- Layer konstruktorn skapar det som beh�vs, s� som descriptor, shader, renderpass, pipeline
	- N�r det som beh�vs har skapats kallas Poly::Renderer::init() f�r att skapa det sista som framebuffers
		(OCH CommandPool med CommandBuffers tills en annan mer dynamisk l�sning hittas)

Varje sak som anv�ndaren beh�ver skapa ska ha Interface klasser f�r att kunna kalla p� create. Det create g�r �r:
	- Skapa ett objekt av den typen som kallas (createShader ex)
	- G�r en init/preInit p� objektet s� att den har det som beh�vs (instance ex)
	- Informera den valda renderern (vulkan renderer) om att den finns s� att den kan ha koll p� den (shader_ptr)
	- Retunera shared_ptr av det �rvda objektet.
*/

class TestLayer : public Poly::Layer
{
public:
	TestLayer() 
	{
		// Poly::Descriptor desc = Poly::Descriptor::create();
		// desc->addBinding(0, 0, Poly::Descriptor::UNIFORM);

		// Poly::Shader shader = Poly::Shader::create();
		// shader->addStage(Poly::Shader::VERTEX, "vertexTest.glsl");

		// Poly::RenderPass renderPass = Poly::RenderPass::create();
		// renderPass->createDefault();

		// Poly::Pipeline pipeline = Poly::Pipeline::create();
		// pipeline->createDefault(Poly::Pipeline::GRAPHICS, desc, shader, renderPass);

		// Poly::Model model = Poly::Model::create(modelPath);

		Poly::RendererAPI::Create(Poly::BackendAPI::VULKAN);
		Poly::RendererAPI::CreateRenderer(Poly::Renderer::TEST);
		pCamera = new Poly::Camera();
		pCamera->SetAspect(1280.f / 720.f);
		pCamera->SetMouseSense(3.f);
		
		Poly::RendererAPI::SetActiveCamera(pCamera);

		Poly::RendererAPI::Init(1280, 720);
	};

	void OnUpdate(Poly::Timestamp dt) override
	{
		//POLY_INFO("Testlayer update!");
		pCamera->Update(dt);
		Poly::RendererAPI::BeginScene();
		Poly::RendererAPI::EndScene();
	};

	void OnDetach() override
	{
		delete pCamera;
	}

private:
	Poly::Camera* pCamera = nullptr;
};

class Sandbox : public Poly::Application
{
public:
	Sandbox()
	{
		PushLayer(new TestLayer());
	}

	~Sandbox()
	{
		Poly::RendererAPI::Shutdown();
	}
};

Poly::Application* Poly::CreateApplication()
{
	return new Sandbox();
}