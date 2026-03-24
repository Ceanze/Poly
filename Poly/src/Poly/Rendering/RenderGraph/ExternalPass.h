#pragma once

#include "Pass.h"
#include "RenderGraphTypes.h"
#include "Poly/Rendering/RenderGraph/ResID.h"

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
		ExternalPass();
		~ExternalPass() override = default;

		virtual PassReflection Reflect() override final { return PassReflection{}; }
		virtual void Execute(const RenderContext&, const RenderData&) override final {}
		virtual void Compile() override final {}

		void RegisterResource(const ResID& guid, const ResourceInfo& info);

		void RemoveResource(const ResID& guid);

		bool HasResource(const ResID& guid) const;

		const std::unordered_map<ResID, ResourceInfo>& GetResources() const;

	private:
		std::unordered_map<ResID, ResourceInfo> m_Resources;
	};
}
