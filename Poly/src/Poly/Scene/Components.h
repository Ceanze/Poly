#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Poly
{
	class Model;
	class Material;

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
		glm::vec3 Scale			= {1.0f, 1.0f, 1.0f};
		glm::quat Orientation;

		glm::mat4 GetTransform()
		{
			return glm::translate(glm::mat4(1.0f), Translation)
				* glm::toMat4(Orientation)
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct MeshComponent
	{
		MeshComponent(Model* pModel, uint32 meshIndex) : pModel(pModel), MeshIndex(meshIndex) {}

		Model* pModel;
		uint32 MeshIndex;
	};

	struct IDComponent
	{
		IDComponent(PolyID polyID) : ID(polyID) {}

		PolyID ID;
	};
}