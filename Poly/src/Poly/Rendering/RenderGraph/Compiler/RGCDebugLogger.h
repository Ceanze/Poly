#pragma once

namespace Poly
{
	struct RGCContext;

	/**
	 * Compiler step that logs the compiled graph state to TRACE for debugging purposes.
	 * Outputs passes in execution order, their incoming connections, pass-specific data
	 * (render pass attachments or sync pass barriers), reflection fields, post-sync
	 * resource states, and debug texture GUIDs.
	 *
	 * Should run after all other compiler steps so it captures the final state.
	 */
	class RGCDebugLogger
	{
	public:
		RGCDebugLogger() = default;

		void Execute(const RGCContext& ctx);
	};
} // namespace Poly
