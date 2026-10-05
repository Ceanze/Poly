#pragma once

namespace Poly::API
{
	struct EntityInfo
	{
		PolyID                   ID     = PolyID::None();
		PolyID                   Parent = PolyID::None();
		std::string              Name; // Empty if the entity has no Name component
		std::vector<PolyID>      Children;
		std::vector<std::string> Components;
	};
} // namespace Poly::API
