#pragma once

#include "Pass.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	/**
	 * Reflect is creating the inputs and outputs of the RenderPass
	 * Execute is called each time the RenderPass should execute, commonly each frame
	 * Compile is called on first-time compilation and recompilation
	 */

	class RenderPass : public Pass
	{
	protected:
		struct RenderPassAttachment
		{
			ETextureLayout	InitalLayout	= ETextureLayout::UNDEFINED;
			ETextureLayout	UsedLayout		= ETextureLayout::UNDEFINED;
			ETextureLayout	FinalLayout		= ETextureLayout::UNDEFINED;
			EFormat			Format			= EFormat::UNDEFINED;
			uint32			Index			= 0;
		};

	public:
		RenderPass() { p_Type = Pass::Type::RENDER; }
		virtual ~RenderPass() = default;

		/**
		 * Informs the RenderGraph about the inputs and outputs of the RenderPass
		 * Called once during re/compilation of the render graph
		 * @return A PassReflection created for this render pass
		 */
		virtual PassReflection Reflect() = 0;

		/**
		 * OPTIONAL
		 * Update the pass, called once per frame before Execute()
		 * Use this to update internal pass resources before Execute()
		 */
		virtual void Update(const RenderContext& context) {};

		/**
		 * Execute the Pass, called once per frame after Update()
		 */
		virtual void Execute(const RenderContext& context, const RenderData& renderData) = 0;

		/**
		 * OPTIONAL
		 * Compile or recompile the Pass
		 * Called once during re/compilation of the render graph
		 */
		virtual void Compile() {};

		void AddAttachment(const std::string& name, ETextureLayout layout, uint32 index, EFormat format)
		{
			p_Attachments[name].UsedLayout = layout;
			p_Attachments[name].InitalLayout = layout;
			p_Attachments[name].FinalLayout = layout;
			p_Attachments[name].Index = index;
			p_Attachments[name].Format = format;
		}

		void SetAttachmentInital(const std::string& name, ETextureLayout layout) { p_Attachments[name].InitalLayout = layout; }
		void SetAttachmentFinal(const std::string& name, ETextureLayout layout) { p_Attachments[name].FinalLayout = layout; }
		void SetAttachmentFormat(const std::string& name, EFormat format) { p_Attachments[name].Format = format; }
		const std::unordered_map<std::string, RenderPassAttachment>& GetAttachments() const { return p_Attachments; };

		void SetDepthStenctilUse(bool isUsingDepthStenctil) { p_usesDepthStencil = isUsingDepthStenctil; }
		bool GetDepthStenctilUse() const { return p_usesDepthStencil; }

	protected:
		friend class RenderGraph;

		std::unordered_map<std::string, RenderPassAttachment> p_Attachments;
		bool p_usesDepthStencil = false;
	};
}