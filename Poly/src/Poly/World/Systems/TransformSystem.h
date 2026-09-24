#pragma once

namespace Poly
{
	class World;

	/*
	 * Resolves the WorldTransformComponent of every entity from its TransformComponent and the transforms of its ancestors,
	 * so systems running after it can use world space transforms without caring about the hierarchy.
	 *
	 * Built-in: every World adds it as its first PostUpdate system, so it does not need to be added manually.
	 */
	class TransformSystem
	{
	public:
		void Update(World& world);
	};
} // namespace Poly
