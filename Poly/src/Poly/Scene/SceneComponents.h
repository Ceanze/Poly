#pragma once

#include <entt/entt.hpp>

struct HierarchyComponent
{
	uint8 ChildrenCount		= 0;
	entt::entity Parent		= entt::null;
	entt::entity First		= entt::null;
	entt::entity Next		= entt::null;
	entt::entity Previous	= entt::null;
};