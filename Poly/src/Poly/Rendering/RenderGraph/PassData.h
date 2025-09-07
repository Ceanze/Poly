#pragma once

#include "Poly/Rendering/RenderGraph/Pass.h"
#include "Poly/Rendering/RenderGraph/Reflection/PassReflection.h"

namespace Poly
{
	struct PassData
	{
		Ref<Pass>		pPass;
		PassReflection	Reflection;
		uint32			NodeIndex;	// Index specified in the RenderGraph, may contain gaps
		uint32			PassIndex;	// Index specified in the RenderGraphProgram, is densely packed

		bool operator== (const PassData& other) const { return NodeIndex == other.NodeIndex; }
	};
}