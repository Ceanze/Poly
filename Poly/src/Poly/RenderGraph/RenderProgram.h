#pragma once

#include "Platform/API/GraphicsPipeline.h"
#include "Poly/RenderGraph/Resource/ResourceState.h"
#include "Poly/RenderGraph/Resource/ResourceType.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <functional>
#include <string>
#include <vector>

namespace Poly
{
	class ExecuteContext;

	struct ResolvedPort
	{
		std::string ShaderName;   // local name in the pass shader
		std::string ResolvedName; // globally resolved name (e.g. "$Color", "SceneAlbedo")
		bool        IsWrite;      // true = output, false = input

		// True if ResolvedName matched a resource registered on the RenderGraph (an externally
		// owned resource supplied per-frame via RenderProgramInstance::UpdateResource()). False
		// means the resource is transient and owned/allocated by the RenderProgramInstance.
		bool IsExternal = false;

		EResourceType  Type         = EResourceType::None;
		FResourceState InitialState = FResourceState::Unknown;

		// Explicit size from the matching IResourceDeclaration::WithSize(), or 0 if unset - in
		// which case the resource is sized to the render target (see RenderProgramInstance).
		uint32 Width  = 0;
		uint32 Height = 0;
	};

	struct ResolvedPass
	{
		std::string               Name;
		std::vector<ResolvedPort> Ports;

		std::vector<std::pair<std::string, FShaderStage>> Shaders;
		GraphicsPipelineDesc                              PipelineDesc;
		std::function<void(ExecuteContext&)>              ExecuteFn;
	};

	class RenderProgram
	{
	public:
		explicit RenderProgram(std::vector<ResolvedPass> sortedPasses);

		const std::vector<ResolvedPass>& GetPasses() const { return m_Passes; }

	private:
		std::vector<ResolvedPass> m_Passes;
	};
} // namespace Poly
