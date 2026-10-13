#include "SlangReflector.h"

#include "polypch.h"

#include <slang/slang.h>

namespace
{
	Poly::FResourceBindPoint SlangTypeToPoly(slang::TypeLayoutReflection* pTypeLayout)
	{
		switch (pTypeLayout->getKind())
		{
		case slang::TypeReflection::Kind::SamplerState:
			return Poly::FResourceBindPoint::SAMPLER;
		case slang::TypeReflection::Kind::ConstantBuffer:
			return Poly::FResourceBindPoint::UNIFORM;
		case slang::TypeReflection::Kind::ShaderStorageBuffer:
			return Poly::FResourceBindPoint::STORAGE;
		case slang::TypeReflection::Kind::Resource:
			break;
		default:
			return Poly::FResourceBindPoint::NONE;
		}

		const SlangResourceShape shape    = pTypeLayout->getResourceShape();
		const bool               writable = pTypeLayout->getResourceAccess() != SLANG_RESOURCE_ACCESS_READ;

		switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
		{
		case SLANG_TEXTURE_1D:
		case SLANG_TEXTURE_2D:
		case SLANG_TEXTURE_3D:
		case SLANG_TEXTURE_CUBE:
			return writable ? Poly::FResourceBindPoint::STORAGE_IMAGE : Poly::FResourceBindPoint::SAMPLED_IMAGE;
		case SLANG_TEXTURE_BUFFER:
			return writable ? Poly::FResourceBindPoint::STORAGE_TEXEL : Poly::FResourceBindPoint::UNIFORM_TEXEL;
		case SLANG_STRUCTURED_BUFFER:
		case SLANG_BYTE_ADDRESS_BUFFER:
			return Poly::FResourceBindPoint::STORAGE;
		case SLANG_TEXTURE_SUBPASS:
			return Poly::FResourceBindPoint::INPUT_ATTACHMENT;
		default:
			return Poly::FResourceBindPoint::NONE;
		}
	}

	// Stage inputs/outputs are often wrapped in structs, the locations of the fields are relative to the struct
	void CollectVaryings(slang::VariableLayoutReflection* pVariable, slang::ParameterCategory category, uint32 baseLocation, std::vector<Poly::ShaderInputOutput>& outVaryings)
	{
		slang::TypeLayoutReflection* pTypeLayout = pVariable->getTypeLayout();

		// skip system values like SV_Position, they do not take up a location
		if (pTypeLayout->getSize(category) == 0)
			return;

		const uint32 location = baseLocation + static_cast<uint32>(pVariable->getOffset(category));

		if (pTypeLayout->getKind() == slang::TypeReflection::Kind::Struct)
		{
			for (unsigned int i = 0; i < pTypeLayout->getFieldCount(); i++)
				CollectVaryings(pTypeLayout->getFieldByIndex(i), category, location, outVaryings);
			return;
		}

		// A value returned directly from the entry point has no name, only a semantic
		const char* pName = pVariable->getName();
		if (!pName)
			pName = pVariable->getSemanticName();

		outVaryings.push_back({pName ? pName : "", location});
	}

	// Confirm the shared bindless slot arrays (see plans/bindless.md) - just reports where
	// textureIndices[]/bufferAddresses[] sit, doesn't map them to resources. They may be nested in a struct
	void FindBindlessSlots(slang::TypeLayoutReflection* pStructLayout, uint32 baseOffset, Poly::ShaderBindlessLayout& outLayout)
	{
		for (unsigned int i = 0; i < pStructLayout->getFieldCount(); i++)
		{
			slang::VariableLayoutReflection* pField       = pStructLayout->getFieldByIndex(i);
			slang::TypeLayoutReflection*     pFieldLayout = pField->getTypeLayout();
			const uint32                     offset       = baseOffset + static_cast<uint32>(pField->getOffset());
			const std::string_view           name         = pField->getName() ? pField->getName() : "";

			if (name == "textureIndices")
			{
				outLayout.HasTextureSlots    = true;
				outLayout.TextureSlotsOffset = offset;
				outLayout.TextureSlotCount   = pFieldLayout->isArray() ? static_cast<uint32>(pFieldLayout->getElementCount()) : 0;
			}
			else if (name == "bufferAddresses")
			{
				outLayout.HasBufferSlots    = true;
				outLayout.BufferSlotsOffset = offset;
				outLayout.BufferSlotCount   = pFieldLayout->isArray() ? static_cast<uint32>(pFieldLayout->getElementCount()) : 0;
			}
			else if (pFieldLayout->getKind() == slang::TypeReflection::Kind::Struct)
			{
				FindBindlessSlots(pFieldLayout, offset, outLayout);
			}
		}
	}
} // namespace

namespace Poly
{
	ShaderReflection SlangReflector::Reflect(slang::ShaderReflection* pLayout)
	{
		ShaderReflection result;

		if (!pLayout)
		{
			POLY_CORE_ERROR("Reflection cannot be created, slang program layout is invalid");
			return result;
		}

		// Inputs and outputs of the stage
		if (pLayout->getEntryPointCount() > 0)
		{
			slang::EntryPointReflection* pEntryPoint = pLayout->getEntryPointByIndex(0);

			for (unsigned int i = 0; i < pEntryPoint->getParameterCount(); i++)
				CollectVaryings(pEntryPoint->getParameterByIndex(i), slang::ParameterCategory::VaryingInput, 0, result.Inputs);

			if (slang::VariableLayoutReflection* pResult = pEntryPoint->getResultVarLayout())
				CollectVaryings(pResult, slang::ParameterCategory::VaryingOutput, 0, result.Outputs);
		}

		// Global parameters: descriptor bindings and push constants
		for (unsigned int i = 0; i < pLayout->getParameterCount(); i++)
		{
			slang::VariableLayoutReflection* pParameter  = pLayout->getParameterByIndex(i);
			slang::TypeLayoutReflection*     pTypeLayout = pParameter->getTypeLayout();
			const char*                      pName       = pParameter->getName() ? pParameter->getName() : "";

			if (pParameter->getCategory() == slang::ParameterCategory::PushConstantBuffer)
			{
				// The block is a ConstantBuffer<T>, the data layout is the one of T
				slang::TypeLayoutReflection* pBlockLayout = pTypeLayout->getKind() == slang::TypeReflection::Kind::ConstantBuffer ? pTypeLayout->getElementTypeLayout() : pTypeLayout;

				result.PushConstants.push_back({pName, static_cast<uint32>(pBlockLayout->getSize()), 0});

				if (pBlockLayout->getKind() == slang::TypeReflection::Kind::Struct)
					FindBindlessSlots(pBlockLayout, 0, result.BindlessLayout);

				continue;
			}

			if (pParameter->getCategory() != slang::ParameterCategory::DescriptorTableSlot)
				continue;

			// An unsized array reports 0 elements
			uint32                       count       = 1;
			slang::TypeLayoutReflection* pLeafLayout = pTypeLayout;
			if (pTypeLayout->isArray())
			{
				count       = static_cast<uint32>(pTypeLayout->getElementCount());
				pLeafLayout = pTypeLayout->getElementTypeLayout();
			}

			// A combined texture sampler (Sampler2D) has the type of a texture, only the binding range tells them apart
			const bool               isCombined = pTypeLayout->getBindingRangeCount() > 0 && pTypeLayout->getBindingRangeType(0) == slang::BindingType::CombinedTextureSampler;
			const FResourceBindPoint bindPoint  = isCombined ? FResourceBindPoint::SAMPLER : SlangTypeToPoly(pLeafLayout);

			result.Bindings.push_back({pName,
			                           pParameter->getBindingSpace(),
			                           pParameter->getBindingIndex(),
			                           bindPoint,
			                           count});
		}

		return result;
	}
} // namespace Poly
