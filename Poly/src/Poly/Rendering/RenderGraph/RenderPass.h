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
		 * @return A PassReflection created for this render pass
		 */
		virtual PassReflection Reflect() = 0;

		/**
		 * Execute the RenderPass
		 */
		virtual void Execute(const RenderContext& context, const RenderData& renderData) = 0;

		/**
		 * OPTIONAL
		 * Compile or recompile the RenderPass
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
		const std::unordered_map<std::string, RenderPassAttachment>& GetAttachments() const { return p_Attachments; };

	protected:
		friend class RenderGraph;

		std::unordered_map<std::string, RenderPassAttachment> p_Attachments;

		// TODO: If/when scenes are added make it a member variable of the renderpass (Ref<Scene>)
	};
}