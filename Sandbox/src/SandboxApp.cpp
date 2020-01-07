#include "Poly.h"

class TestLayer : public Poly::Layer
{
public:
	TestLayer() {};

	void onUpdate() override
	{
		POLY_INFO("Testlayer update!");
	};
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

	}
};

Poly::Application* Poly::CreateApplication()
{
	return new Sandbox();
}