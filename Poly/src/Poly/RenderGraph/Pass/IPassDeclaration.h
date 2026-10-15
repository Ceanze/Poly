#pragma once

#include "BlendMode.h"
#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/RenderGraph/Resource/ResourceState.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <functional>
#include <string_view>

namespace Poly
{
	class SetupContext;
	class ExecuteContext;
	class PassDeclaration;

	class IPassDeclaration
	{
	public:
		virtual ~IPassDeclaration() = default;

		/*
		 * Adds a stage of a .slang shader to the pass. Multiple stages can be added, from the same file or not, but only one per stage.
		 * @param shaderPath The path of the .slang file.
		 * @param stage The stage to use, the file must have exactly one entry point marked [shader("...")] for it.
		 */
		virtual IPassDeclaration& WithShader(std::string_view shaderPath, FShaderStage stage) = 0;

		/*
		 * Add a setup function executed when the render program is compiled.
		 */
		virtual IPassDeclaration& WithSetupFn(std::function<void(SetupContext&)> setupFn) = 0;

		/*
		 * Declares which queue this pass should execute on. Defaults to FQueueType::GRAPHICS if never called.
		 *
		 * @param queue The queue to run this pass on.
		 * @return A reference to this for chaining.
		 */
		virtual IPassDeclaration& OnQueue(FQueueType queue) = 0;

		/*
		 * Add an execute function called every frame during render program execution.
		 * A pass with an execute function cannot be saved to file, prefer WithExecutor(). Replaces the executor of the pass.
		 */
		virtual IPassDeclaration& WithExecuteFn(std::function<void(ExecuteContext&)> executeFn) = 0;

		/*
		 * Uses a registered executor as the execute function of the pass, see ExecutorRegistry. The executor has to be
		 * registered by the time the render plan is built. Replaces the execute function of the pass.
		 * @param executorName The name of the executor.
		 * @param paramsJson The parameters given to the executor, as a json object. Empty for none.
		 * @return A reference to this for chaining.
		 */
		virtual IPassDeclaration& WithExecutor(std::string_view executorName, std::string_view paramsJson = "") = 0;

		// --- Pipeline state ---
		// Everything below is optional. A pass that sets none of it draws filled, back face culled, counter clockwise
		// triangle lists without blending, and depth tests (LESS_OR_EQUAL) and writes if it maps EFeaturePort::Depth.

		/*
		 * Sets how vertices are assembled into primitives. Defaults to ETopology::TRIANGLE_LIST.
		 */
		virtual IPassDeclaration& WithTopology(ETopology topology) = 0;

		/*
		 * Sets how polygons are rasterized. Defaults to EPolygonMode::FILL.
		 */
		virtual IPassDeclaration& WithPolygonMode(EPolygonMode mode) = 0;

		/*
		 * Sets which faces are culled. Defaults to ECullMode::BACK.
		 */
		virtual IPassDeclaration& WithCullMode(ECullMode mode) = 0;

		/*
		 * Sets the winding order of front faces. Defaults to counter clockwise.
		 */
		virtual IPassDeclaration& WithFrontFaceClockwise(bool clockwise = true) = 0;

		/*
		 * Overrides depth testing. Defaults to enabled if the pass maps EFeaturePort::Depth, otherwise disabled.
		 */
		virtual IPassDeclaration& WithDepthTest(bool enable) = 0;

		/*
		 * Overrides depth writing. Defaults to following the depth test.
		 */
		virtual IPassDeclaration& WithDepthWrite(bool enable) = 0;

		/*
		 * Sets the depth comparison. Defaults to ECompareOp::LESS_OR_EQUAL.
		 */
		virtual IPassDeclaration& WithDepthCompareOp(ECompareOp op) = 0;

		/*
		 * Enables depth bias, e.g. for shadow maps.
		 * @param constantFactor Constant depth added to each fragment.
		 * @param slopeFactor Depth added to each fragment scaled by its slope.
		 * @param clamp Maximum (or minimum, if negative) bias of a fragment, 0 for no clamp.
		 */
		virtual IPassDeclaration& WithDepthBias(float constantFactor, float slopeFactor, float clamp = 0.0f) = 0;

		/*
		 * Sets how the pass's color output is blended with the attachment. Defaults to EBlendMode::Opaque.
		 */
		virtual IPassDeclaration& WithBlend(EBlendMode mode) = 0;

		/*
		 * Sets a custom blend state for the pass's color output, for when no EBlendMode fits.
		 */
		virtual IPassDeclaration& WithBlend(const ColorBlendAttachmentDesc& blend) = 0;

		/*
		 * Declares the vertex buffer the pass binds. Only needed together with WithVertexAttribute, passes that
		 * read their vertices from a mapped buffer (vertex pulling) have no vertex layout.
		 * @param stride The size of one vertex, in bytes.
		 * @param inputRate Whether the buffer advances per vertex or per instance.
		 */
		virtual IPassDeclaration& WithVertexLayout(uint32 stride, EVertexInputRate inputRate = EVertexInputRate::VERTEX) = 0;

		/*
		 * Adds a vertex attribute read from the vertex buffer declared by WithVertexLayout.
		 * @param location The location of the input in the vertex shader.
		 * @param format The format of the attribute in the vertex buffer.
		 * @param offset The offset of the attribute from the start of a vertex, in bytes.
		 */
		virtual IPassDeclaration& WithVertexAttribute(uint32 location, EFormat format, uint32 offset) = 0;

		/*
		 * Escape hatch for pipeline state without a setter (stencil, logic op, static viewport/scissor, ...).
		 * Called with the fully resolved pipeline description when the render program is built, after every
		 * default and setter has been applied.
		 */
		virtual IPassDeclaration& WithPipelineOverride(std::function<void(GraphicsPipelineDesc&)> overrideFn) = 0;

		/*
		 * Maps a feature port to a shader resource of the pass. This works as a connection between the stricter feature pipeline
		 * and the more flexible pass pipeline. Mapping a feature port means that it is then exposed to other features in the pipeline,
		 * as a read if the port is an input, or a write if it is an output.
		 *
		 * @param port The feature port to map.
		 * @param shaderResourceName The name of the shader resource to map to. This should match a resource in the shader of the pass.
		 * @param loadOp Overrides the auto-inferred attachment load op (first write in the program clears, later writes load).
		 *               Leave at ELoadOp::NONE to keep the automatic behavior.
		 * @return A reference to this for chaining.
		 */
		virtual PassDeclaration& MapResource(EFeaturePort port, std::string_view shaderResourceName, ELoadOp loadOp = ELoadOp::NONE) = 0;

		/*
		 * Maps a global variable to a shader global of the pass.
		 *
		 * @param globalName The name of the global variable to map.
		 * @param shaderGlobalName The name of the shader global to map to. This should match a global in the shader of the pass.
		 * @return A reference to this for chaining.
		 */
		virtual PassDeclaration& MapGlobal(std::string_view globalName, std::string_view shaderGlobalName) = 0;

		/*
		 * Imports a resource to the pass. This is different to mapping feature ports, as it isn't limited to the available feature ports.
		 * Importing a resource means that the pass can have a dependency to a resource from another pass, without the need of a feature port.
		 * For the importing to be valid at compilation, another feature must export the same resource name before this pass.
		 * @param resourceName The name of the resource to import. This should match the exported resource name of another pass.
		 * @param shaderResourceName The name of the shader resource to map to. This should match a resource in the shader of the pass.
		 * @return A reference to this for chaining
		 */
		virtual PassDeclaration& ImportResource(std::string_view resourceName, std::string_view shaderResourceName) = 0;

		/*
		 * Exports a resource from the pass. This is different to mapping feature ports, as it isn't limited to the available feature ports.
		 * Exporting a resource means that the pass can have a dependency to a resource from another pass, without the need of a feature port.
		 * For the exporting to be valid at compilation, another pass must import the same resource name after this pass.
		 * @param resourceName The name of the resource to export. This should match the imported resource name of another pass.
		 * @param shaderResourceName The name of the shader resource to map to. This should match a resource in the shader of the pass.
		 * @return A reference to this for chaining
		 */
		virtual PassDeclaration& ExportResource(std::string_view resourceName, std::string_view shaderResourceName) = 0;

		/*
		 * Declares that the pass reads a resource without binding it to a shader variable (e.g. an indirect argument
		 * buffer). This gives the graph the dependency and the barrier information. Use MapGlobal for resources the shader reads.
		 * The resource must be registered on the render graph. Resolve it in the execute function with ExecuteContext::GetBuffer.
		 * @param resourceName The global name of the resource, as registered on the render graph.
		 * @param state The state the resource must be in while the pass reads it, e.g. FResourceState::IndirectArgument.
		 * @return A reference to this for chaining
		 */
		virtual PassDeclaration& ReadResource(std::string_view resourceName, FResourceState state) = 0;

		/*
		 * Declares that the pass writes a resource without binding it to a shader variable (e.g. a copy destination).
		 * The resource must be registered on the render graph. See ReadResource.
		 * @param resourceName The global name of the resource, as registered on the render graph.
		 * @param state The state the resource must be in while the pass writes it, e.g. FResourceState::CopyDest.
		 * @return A reference to this for chaining
		 */
		virtual PassDeclaration& WriteResource(std::string_view resourceName, FResourceState state) = 0;
	};
} // namespace Poly
