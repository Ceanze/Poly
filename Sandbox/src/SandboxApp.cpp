#include "Poly.h"

/*
Steg för skapning av ett enkelt PVK program:
	- Application kallar på Renderer::preInit() som skapar Window, Instance, SwapChain, syncobjects
	- Layer konstruktorn skapar det som behövs, så som descriptor, shader, renderpass, pipeline
	- När det som behövs har skapats kallas Poly::Renderer::init() för att skapa det sista som framebuffers
		(OCH CommandPool med CommandBuffers tills en annan mer dynamisk lösning hittas)

Varje sak som användaren behöver skapa ska ha Interface klasser för att kunna kalla på create. Det create gör är:
	- Skapa ett objekt av den typen som kallas (createShader ex)
	- Gör en init/preInit på objektet så att den har det som behövs (instance ex)
	- Informera den valda renderern (vulkan renderer) om att den finns så att den kan ha koll på den (shader_ptr)
	- Retunera shared_ptr av det ärvda objektet.
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

		Poly::RendererAPI::create(Poly::BackendAPI::VULKAN);
		Poly::RendererAPI::createRenderer(Poly::Renderer::TEST);
		this->camera = new Poly::Camera();
		this->camera->setAspect(1280.f / 720.f);
		this->camera->setMouseSense(3.f);
		
		Poly::RendererAPI::setActiveCamera(this->camera);

		Poly::RendererAPI::init(1280, 720);
	};

	void onUpdate(float dt) override
	{
		//POLY_INFO("Testlayer update!");
		this->camera->update(dt);
		Poly::RendererAPI::beginScene();
		Poly::RendererAPI::endScene();
	};

	void onDetach() override
	{
		delete this->camera;
	}

private:
	Poly::Camera* camera = nullptr;
};

class Sandbox : public Poly::Application
{
public:
	Sandbox()
	{
		pushLayer(new TestLayer());
	}

	~Sandbox()
	{
		Poly::RendererAPI::shutdown();
	}
};

Poly::Application* Poly::CreateApplication()
{
	return new Sandbox();
}