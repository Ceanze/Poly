#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Poly
{
	class Mesh;

	struct HierarchyComponent
	{
		uint8 ChildrenCount		= 0;
		entt::entity Parent		= entt::null;
		entt::entity First		= entt::null;
		entt::entity Next		= entt::null;
		entt::entity Previous	= entt::null;
	};

	struct TransformComponent
	{
		glm::vec3 Translation	= {0.0f, 0.0f, 0.0f};
		glm::vec3 Rotation		= {0.0f, 0.0f, 0.0f};
		glm::vec3 Scale			= {1.0f, 1.0f, 1.0f};
	};

	struct MeshComponent
	{
		Ref<Mesh> pMesh;
		PolyID MaterialID;
	};
}