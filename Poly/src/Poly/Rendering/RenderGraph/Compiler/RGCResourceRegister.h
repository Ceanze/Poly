#pragma once

#include <Poly/Rendering/RenderGraph/PassResID.h>

namespace Poly
{
	struct RGCContext;
	struct CompiledPass;

	class RGCResourceRegister
	{
	public:
		RGCResourceRegister() = default;

		void Execute(RGCContext& ctx);

	private:
		void      RegisterExternalResources(RGCContext& ctx);
		void      RegisterResources(RGCContext& ctx);
		void      AliasInputs(RGCContext& ctx, CompiledPass& compiledPass);
		void      RegisterOutputs(RGCContext& ctx, CompiledPass& compiledPass);
		PassResID GetAliasedPassResID(RGCContext& ctx, CompiledPass& compiledPass, const PassResID& inputID);
	};
} // namespace Poly