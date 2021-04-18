#pragma once

namespace Poly
{
	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		void AddModel(PolyID model);

		void RemoveModel(PolyID model);

		void Draw();

	private:
		std::vector<PolyID> m_Models;
	};
}