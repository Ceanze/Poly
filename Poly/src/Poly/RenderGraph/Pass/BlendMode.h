#pragma once

#include "Platform/API/GraphicsPipeline.h"

namespace Poly
{
	enum class EBlendMode
	{
		Opaque,             // No blending, the output replaces the attachment
		AlphaBlend,         // Straight alpha: src * srcAlpha + dst * (1 - srcAlpha)
		PremultipliedAlpha, // Color already multiplied by alpha: src + dst * (1 - srcAlpha)
		Additive            // src * srcAlpha + dst
	};

	/*
	 * Converts a blend mode to the blend state of a color attachment, writing all color components.
	 * @param mode The blend mode to convert.
	 * @return The equivalent color blend attachment description.
	 */
	inline ColorBlendAttachmentDesc ToBlendDesc(EBlendMode mode)
	{
		ColorBlendAttachmentDesc desc = {};
		switch (mode)
		{
		case EBlendMode::AlphaBlend:
			desc.BlendEnable         = true;
			desc.SrcColorBlendFactor = EBlendFactor::SRC_ALPHA;
			desc.DstColorBlendFactor = EBlendFactor::ONE_MINUS_SRC_ALPHA;
			desc.SrcAlphaBlendFactor = EBlendFactor::ONE;
			desc.DstAlphaBlendFactor = EBlendFactor::ONE_MINUS_SRC_ALPHA;
			break;
		case EBlendMode::PremultipliedAlpha:
			desc.BlendEnable         = true;
			desc.SrcColorBlendFactor = EBlendFactor::ONE;
			desc.DstColorBlendFactor = EBlendFactor::ONE_MINUS_SRC_ALPHA;
			desc.SrcAlphaBlendFactor = EBlendFactor::ONE;
			desc.DstAlphaBlendFactor = EBlendFactor::ONE_MINUS_SRC_ALPHA;
			break;
		case EBlendMode::Additive:
			desc.BlendEnable         = true;
			desc.SrcColorBlendFactor = EBlendFactor::SRC_ALPHA;
			desc.DstColorBlendFactor = EBlendFactor::ONE;
			desc.SrcAlphaBlendFactor = EBlendFactor::ONE;
			desc.DstAlphaBlendFactor = EBlendFactor::ONE;
			break;
		case EBlendMode::Opaque:
		default:
			break;
		}

		return desc;
	}
} // namespace Poly
