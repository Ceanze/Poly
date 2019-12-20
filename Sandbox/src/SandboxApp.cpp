#include "Poly.h"

class Sandbox : public Poly::Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}
};

Poly::Application* Poly::CreateApplication()
{
	return new Sandbox();
}