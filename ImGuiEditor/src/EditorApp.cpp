#include "EditorLayer.h"

#include <Poly/Core/Application.h>
#include <Poly/Core/Engine.h>
#include <Poly/World/Serialization/WorldSerializer.h>

namespace Editor
{
	class Application : public Poly::Application
	{
	public:
		explicit Application(std::string_view worldPath)
		    : m_WorldPath(worldPath)
		{}

		void OnInit() override { PushLayer(new EditorLayer(m_WorldPath)); }

	private:
		std::string m_WorldPath;

		std::optional<Poly::Window::Properties> GetWindowProperties() const override { return Poly::Window::Properties{1600, 900, "Poly Editor"}; }
	};
} // namespace Editor
