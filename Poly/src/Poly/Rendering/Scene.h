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

		const std::vector<PolyID>& GetModels() const { return m_Models; }

		uint32 GetModelCount() const { return m_Models.size(); }

	private:
		std::vector<PolyID> m_Models;
	};
}