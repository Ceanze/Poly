#include "RenderPlanBuilder.h"

#include "Feature/FeatureDeclaration.h"
#include "Feature/FeaturePort.h"
#include "Pass/PassDeclaration.h"
#include "Poly/Resources/Shader/ShaderManager.h"
#include "Resource/ResourceDeclaration.h"
#include "Resource/ResourceUsage.h"
#include "SetupContext.h"

#include <algorithm>

namespace Poly
{
	ViewStageBuilder::ViewStageBuilder(RenderPlanBuilder& planBuilder, size_t entryIndex)
	    : m_PlanBuilder(planBuilder)
	    , m_EntryIndex(entryIndex)
	{}

	ViewStageBuilder& ViewStageBuilder::AddFeature(std::string_view name)
	{
		m_PlanBuilder.m_Entries[m_EntryIndex].Features.emplace_back(name);
		return *this;
	}

	RenderPlanBuilder& ViewStageBuilder::EndStage()
	{
		return m_PlanBuilder;
	}

	RenderPlanBuilder::RenderPlanBuilder(Ref<RenderCatalog> catalog)
	    : m_Catalog(std::move(catalog))
	{}

	RenderPlanBuilder& RenderPlanBuilder::AddFrameFeature(std::string_view name)
	{
		m_Entries.push_back({RenderPlanEntry::EType::FrameFeature, std::string(name), {std::string(name)}});
		return *this;
	}

	ViewStageBuilder RenderPlanBuilder::AddViewStage(std::string_view name)
	{
		m_Entries.push_back({RenderPlanEntry::EType::ViewStage, std::string(name), {}});
		return ViewStageBuilder(*this, m_Entries.size() - 1);
	}

	Ref<RenderPlan> RenderPlanBuilder::Build()
	{
		std::vector<RenderPlanEntry>            entries;
		std::unordered_map<std::string, size_t> featureInstanceCount;

		for (const Entry& entry : m_Entries)
		{
			const bool isStage = entry.Type == RenderPlanEntry::EType::ViewStage;

			if (isStage && std::ranges::any_of(entries, [&entry](const RenderPlanEntry& other) { return other.Type == RenderPlanEntry::EType::ViewStage && other.Name == entry.Name; }))
			{
				POLY_CORE_ERROR("Render plan already has a view stage named '{}', the second one is left out.", entry.Name);
				continue;
			}

			RenderPlanEntry planEntry = {.Type = entry.Type, .Name = entry.Name};
			if (!FlattenFeatures(entry, featureInstanceCount, planEntry.Passes))
				continue;

			if (isStage)
			{
				const bool writesColor = std::ranges::any_of(planEntry.Passes, [](const ResolvedPass& pass) {
					return std::ranges::any_of(pass.Ports, [](const ResolvedPort& port) { return port.IsWrite && port.Semantic == EFeaturePort::Color; });
				});

				if (!writesColor)
				{
					POLY_CORE_ERROR("View stage '{}' has no feature writing EFeaturePort::Color, it is left out of the render plan.", entry.Name);
					continue;
				}
			}

			AssignBindlessSlots(planEntry.Passes);
			entries.push_back(std::move(planEntry));
		}

		return CreateRef<RenderPlan>(std::move(entries));
	}

	// Expands features into an ordered flat list of passes. Port names are left unscoped, they are scoped once
	// it is known which views the passes are rendered for (RenderProgramBuilder). Import/export resource names
	// are prefixed per feature instance ("featureName#N.resName") so that two uses of the same feature don't
	// share internal resource connections.
	bool RenderPlanBuilder::FlattenFeatures(const Entry& entry, std::unordered_map<std::string, size_t>& featureInstanceCount, std::vector<ResolvedPass>& outPasses) const
	{
		const bool isStage = entry.Type == RenderPlanEntry::EType::ViewStage;

		for (const auto& featureName : entry.Features)
		{
			const FeatureDeclaration* feature = m_Catalog->GetFeatureRegistry().Get(featureName);
			if (!feature)
			{
				POLY_CORE_ERROR("Feature '{}' not found in the render catalog, '{}' is left out of the render plan.", featureName, entry.Name);
				return false;
			}

			const size_t      instanceIdx   = featureInstanceCount[featureName]++;
			const std::string featurePrefix = featureName + "#" + std::to_string(instanceIdx) + ".";

			for (const auto& passName : feature->GetPasses())
			{
				PassDeclaration* pass = m_Catalog->GetPassRegistry().Get(passName);
				if (!pass)
				{
					POLY_CORE_ERROR("Pass '{}' referenced by feature '{}' not found in the render catalog, '{}' is left out of the render plan.",
					                passName, featureName, entry.Name);
					return false;
				}

				SetupContext setupCtx(*pass);
				pass->CallSetupFn(setupCtx);

				ResolvedPass resolved;
				resolved.Name         = passName;
				resolved.Shaders      = pass->GetShaders();
				resolved.PipelineDesc = pass->BuildPipelineDesc();
				resolved.ExecuteFn    = pass->GetExecuteFn();
				resolved.Queue        = pass->GetQueue();

				// Semantic ports. Depth and stencil belong to the view, the color target is resolved per view
				// by the RenderProgramBuilder since several views can share one.
				for (const ResourceMapping& mapping : pass->GetResourceMappings())
				{
					ResolvedPort port;
					port.ShaderName     = mapping.ShaderResourceName;
					port.Name           = std::string(ToSemanticName(mapping.Port));
					port.IsWrite        = true;
					port.Scope          = EResourceScope::View;
					port.Semantic       = mapping.Port;
					port.LoadOpOverride = mapping.LoadOpOverride;
					resolved.Ports.push_back(std::move(port));
				}

				const size_t firstNamedPort = resolved.Ports.size();

				for (const auto& [globalName, shaderName] : pass->GetGlobalMappings())
					resolved.Ports.push_back({.ShaderName = shaderName, .Name = globalName, .IsWrite = false});

				const size_t firstFeaturePort = resolved.Ports.size();

				for (const auto& [resName, shaderName] : pass->GetImportedResources())
					resolved.Ports.push_back({.ShaderName = shaderName, .Name = featurePrefix + resName, .IsWrite = false, .Scope = EResourceScope::View});

				for (const auto& [resName, shaderName] : pass->GetExportedResources())
					resolved.Ports.push_back({.ShaderName = shaderName, .Name = featurePrefix + resName, .IsWrite = true, .Scope = EResourceScope::View});

				const size_t firstUsePort = resolved.Ports.size();

				// Read/WriteResource ports: graph dependency + barrier state only, no shader binding (empty ShaderName).
				// These must reference a registered resource, otherwise the name would silently fall through to a
				// transient render-target-sized texture below.
				for (const ResourceUse& use : pass->GetResourceUses())
				{
					if (!m_Catalog->GetResourceRegistry().Get(use.Name))
					{
						POLY_CORE_ERROR("Pass '{}' uses resource '{}' via Read/WriteResource, but it is not registered in the render catalog; skipping.",
						                passName, use.Name);
						continue;
					}

					ResolvedPort port;
					port.Name       = use.Name;
					port.IsWrite    = use.IsWrite;
					port.UsageState = use.State;
					resolved.Ports.push_back(std::move(port));
				}

				// A port whose name matches a resource registered in the catalog with an explicit size is
				// graph-owned/allocated at that fixed size (e.g. a shadow map); one registered without a size is
				// externally owned and supplied through the RenderResourceTable of its scope. Everything
				// unregistered is transient and sized to the render target.
				for (size_t i = firstNamedPort; i < resolved.Ports.size(); i++)
				{
					ResolvedPort& port = resolved.Ports[i];

					const ResourceDeclaration* resDecl = m_Catalog->GetResourceRegistry().Get(port.Name);
					if (!resDecl)
						continue;

					port.ResourceType = resDecl->GetResourceType();
					port.InitialState = resDecl->GetInitialState();
					port.Width        = resDecl->GetWidth();
					port.Height       = resDecl->GetHeight();
					port.IsExternal   = !resDecl->HasSize();

					// Imports and exports stay view scoped whatever a resource with the same name says
					if (i < firstFeaturePort || i >= firstUsePort)
						port.Scope = resDecl->GetScope();

					if (!isStage && port.Scope != EResourceScope::Global)
					{
						POLY_CORE_ERROR("Pass '{}' of frame feature '{}' uses resource '{}', which is not of global scope. A frame feature has no view "
						                "and no world, the feature is left out of the render plan.",
						                passName, featureName, port.Name);
						return false;
					}
				}

				outPasses.push_back(std::move(resolved));
			}
		}

		return true;
	}

	void RenderPlanBuilder::AssignBindlessSlots(std::vector<ResolvedPass>& passes) const
	{
		for (ResolvedPass& pass : passes)
		{
			if (pass.Shaders.empty())
				continue;

			ShaderBindlessLayout layout;
			for (const auto& [shaderPath, shaderStage] : pass.Shaders)
			{
				const PolyID                shaderID    = ShaderManager::CreateShader(shaderPath, shaderStage);
				const ShaderData&           shaderData  = ShaderManager::GetShader(shaderID);
				const ShaderBindlessLayout& stageLayout = shaderData.Reflection.BindlessLayout;

				if (stageLayout.HasTextureSlots)
				{
					if (layout.HasTextureSlots && (layout.TextureSlotsOffset != stageLayout.TextureSlotsOffset ||
					                               layout.TextureSlotCount != stageLayout.TextureSlotCount))
						POLY_CORE_ERROR("Pass '{}' shader stages disagree on textureIndices[] layout ({} slots @ offset {} vs {} slots @ offset {}).",
						                pass.Name, layout.TextureSlotCount, layout.TextureSlotsOffset,
						                stageLayout.TextureSlotCount, stageLayout.TextureSlotsOffset);

					layout.HasTextureSlots    = true;
					layout.TextureSlotsOffset = stageLayout.TextureSlotsOffset;
					layout.TextureSlotCount   = stageLayout.TextureSlotCount;
				}

				if (stageLayout.HasBufferSlots)
				{
					if (layout.HasBufferSlots && (layout.BufferSlotsOffset != stageLayout.BufferSlotsOffset ||
					                              layout.BufferSlotCount != stageLayout.BufferSlotCount))
						POLY_CORE_ERROR("Pass '{}' shader stages disagree on bufferAddresses[] layout ({} slots @ offset {} vs {} slots @ offset {}).",
						                pass.Name, layout.BufferSlotCount, layout.BufferSlotsOffset,
						                stageLayout.BufferSlotCount, stageLayout.BufferSlotsOffset);

					layout.HasBufferSlots    = true;
					layout.BufferSlotsOffset = stageLayout.BufferSlotsOffset;
					layout.BufferSlotCount   = stageLayout.BufferSlotCount;
				}

				for (const auto& pushConstant : shaderData.Reflection.PushConstants)
					pass.PushConstantSize = std::max(pass.PushConstantSize, pushConstant.Offset + pushConstant.Size);
			}

			if (!layout.HasTextureSlots && !layout.HasBufferSlots)
				continue;

			pass.BufferSlotsOffset  = layout.BufferSlotsOffset;
			pass.TextureSlotsOffset = layout.TextureSlotsOffset;

			for (uint32 portIndex = 0; portIndex < pass.Ports.size(); portIndex++)
			{
				const ResolvedPort& port = pass.Ports[portIndex];

				// Read/WriteResource ports have no shader variable, so they don't occupy a bindless slot.
				if (port.ShaderName.empty() && port.UsageState != FResourceState::Unknown)
					continue;

				if (IsTextureResourceType(port.ResourceType))
				{
					if (!layout.HasTextureSlots)
						continue;

					const uint32 slot = static_cast<uint32>(pass.TextureSlots.size());
					if (slot >= layout.TextureSlotCount)
					{
						POLY_CORE_ERROR("Pass '{}' has more texture resources ({}) than its shader's textureIndices[] array supports ({}); '{}' was not assigned a slot.",
						                pass.Name, slot + 1, layout.TextureSlotCount, port.Name);
						continue;
					}

					pass.TextureSlots.push_back({slot, portIndex});
				}
				else if (IsBufferResourceType(port.ResourceType))
				{
					if (!layout.HasBufferSlots)
						continue;

					const uint32 slot = static_cast<uint32>(pass.BufferSlots.size());
					if (slot >= layout.BufferSlotCount)
					{
						POLY_CORE_ERROR("Pass '{}' has more buffer resources ({}) than its shader's bufferAddresses[] array supports ({}); '{}' was not assigned a slot.",
						                pass.Name, slot + 1, layout.BufferSlotCount, port.Name);
						continue;
					}

					pass.BufferSlots.push_back({slot, portIndex});
				}
			}
		}
	}
} // namespace Poly
