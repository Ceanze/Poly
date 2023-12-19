#include "RenderPass.h"

namespace Poly
{
    void RenderPass::AddAttachment(const std::string& name, ETextureLayout layout, uint32 index, EFormat format)
    {
        AdaptAttachmentsSize(index);
        auto& attachment = p_Attachments[index];

        attachment.Name         = name;
        attachment.UsedLayout   = layout;
        attachment.InitalLayout = layout;
        attachment.FinalLayout  = layout;
        attachment.Index        = index;
        attachment.Format       = format;
    }

    void RenderPass::SetAttachmentInital(const std::string& name, ETextureLayout layout)
    {
        const auto attachment = GetAttachment(name);
        if (!attachment)
        {
            POLY_CORE_WARN("SetAttachmentInitial was called with name {} - but that name has never been added using AddAttachment", name);
            return;
        }

        attachment->get().InitalLayout = layout;
    }

    void RenderPass::SetAttachmentFinal(const std::string& name, ETextureLayout layout)
    {
        const auto attachment = GetAttachment(name);
        if (!attachment)
        {
            POLY_CORE_WARN("SetAttachmentFinal was called with name {} - but that name has never been added using AddAttachment", name);
            return;
        }

        attachment->get().FinalLayout = layout;
    }

    void RenderPass::SetAttacœhmentFormat(const std::string& name, EFormat format)
    {
        const auto attachment = GetAttachment(name);
        if (!attachment)
        {
            POLY_CORE_WARN("SetAttachmentFormat was called with name {} - but that name has never been added using AddAttachment", name);
            return;
        }

        attachment->get().Format = format;
    }

    void RenderPass::AdaptAttachmentsSize(uint32 index)
    {
        if (index >= p_Attachments.size())
            p_Attachments.resize(index + 1);
    }

    std::optional<std::reference_wrapper<RenderPass::RenderPassAttachment>> RenderPass::GetAttachment(const std::string& name)
    {
        const auto itr = std::find_if(p_Attachments.begin(), p_Attachments.end(), [name](const RenderPassAttachment& attachment)
        {
            return attachment.Name == name;
        });

        if (itr == p_Attachments.end())
            return std::nullopt;

        return *itr;
    }
}
