#pragma once
#include "Pass.h"
#include "RenderGraphTypes.h"

namespace Poly
{
	/**
	 * ExternalPass is a virtual, non-executable node that represents the "$" namespace
	 * in the render graph. External resources are registered on this pass and real graph
	 * edges are created from it to any consuming pass.
	 */
	class ExternalPass : public Pass
	{
	public:
		ExternalPass()
		{
			p_Type = Pass::Type::EXTERNAL;
			p_Name = "$";
		}
		~ExternalPass() = default;

		virtual PassReflection Reflect() override final { return PassReflection{}; }
		virtual void Execute(const RenderContext&, const RenderData&) override final {}
		virtual void Compile() override final {}

		void RegisterResource(const ResourceGUID& guid, const ResourceInfo& info)
		{
			m_Resources[guid] = info;
		}

		void RemoveResource(const ResourceGUID& guid)
		{
			m_Resources.erase(guid);
		}

		bool HasResource(const ResourceGUID& guid) const
		{
			return m_Resources.contains(guid);
		}

		const std::unordered_map<ResourceGUID, ResourceInfo, ResourceGUIDHasher>& GetResources() const
		{
			return m_Resources;
		}

	private:
		std::unordered_map<ResourceGUID, ResourceInfo, ResourceGUIDHasher> m_Resources;
	};
}
