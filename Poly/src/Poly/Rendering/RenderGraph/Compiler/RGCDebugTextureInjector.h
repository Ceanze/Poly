#pragma once

namespace Poly
{
	struct RGCContext;

	/**
	 * Compiler step that inserts a "DebugTextureSync" pass before the first debug consumer
	 * (e.g. ImGuiPass) to transition all texture outputs to SHADER_READ_ONLY_OPTIMAL so they
	 * can be sampled freely via ImGui::Image().
	 *
	 * Only active when RenderGraphDefaultParams::EnableDebugTextures is true.
	 * Must run after RGCSynchroniser (requires PostSyncResourceStates).
	 */
	class RGCDebugTextureInjector
	{
	public:
		RGCDebugTextureInjector() = default;

		void Execute(RGCContext& ctx);
	};
} // namespace Poly
