#include "EditorLayer.h"

#include <Poly.h>

namespace Editor
{
	class Application : public Poly::Application
	{
	public:
		void OnInit() override { PushLayer(new EditorLayer("")); }

	private:
		std::optional<Poly::Window::Properties> GetWindowProperties() const override { return Poly::Window::Properties{1600, 900, "Poly Editor"}; }
	};
} // namespace Editor

Poly::Application* Poly::CreateApplication()
{
	return new Editor::Application();
}
