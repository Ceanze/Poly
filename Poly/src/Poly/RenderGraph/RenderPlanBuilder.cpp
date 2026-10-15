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
		m_PlanBuilder.m_Desc.Entries[m_EntryIndex].Features.emplace_back(name);
		return *this;
	}

	RenderPlanBuilder& ViewStageBuilder::EndStage()
	{
		return m_PlanBuilder;
	}

	RenderPlanBuilder::RenderPlanBuilder(Ref<RenderCatalog> catalog)
	    : m_Catalog(std::move(catalog))
	{}

	RenderPlanBuilder::RenderPlanBuilder(Ref<RenderCatalog> catalog, RenderPlanDesc desc)
	    : m_Catalog(std::move(catalog))
	    , m_Desc(std::move(desc))
	{}

	RenderPlanBuilder& RenderPlanBuilder::AddFrameFeature(std::string_view name)
	{
		m_Desc.Entries.push_back({RenderPlanEntry::EType::FrameFeature, std::string(name), {std::string(name)}});
		return *this;
	}

	ViewStageBuilder RenderPlanBuilder::AddViewStage(std::string_view name)
	{
		m_Desc.Entries.push_back({RenderPlanEntry::EType::ViewStage, std::string(name), {}});
		return ViewStageBuilder(*this, m_Desc.Entries.size() - 1);
	}

	Ref<RenderPlan> RenderPlanBuilder::Build()
	{
		std::vector<RenderPlanEntry>            entries;
		std::unordered_map<std::string, size_t> featureInstanceCount;

		for (const RenderPlanEntryDesc& entry : m_Desc.Entries)
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

			ResolveBindlessBindings(planEntry.Passes);
			entries.push_back(std::move(planEntry));
		}

		return CreateRef<RenderPlan>(std::move(entries));
	}

	// Expands features into an ordered flat list of passes. Port names are left unscoped, they are scoped once
	// it is known which views the passes are rendered for (RenderProgramBuilder). Import/export resource names
	// are prefixed per feature instance ("featureName#N.resName") so that two uses of the same feature don't
	// share internal resource connections.
	bool RenderPlanBuilder::FlattenFeatures(const RenderPlanEntryDesc& entry, std::unordered_map<std::string, size_t>& featureInstanceCount, std::vector<ResolvedPass>& outPasses) const
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
				resolved.Queue        = pass->GetQueue();

				const PassExecutor& executor = pass->GetDesc().Executor;
				if (executor.Name.empty())
				{
					resolved.ExecuteFn = pass->GetExecuteFn();
				}
				else
				{
					Result<ExecuteFn> executeFn = m_Catalog->GetExecutorRegistry().Create(executor.Name, executor.ParamsJson);
					if (!executeFn)
					{
						POLY_CORE_ERROR("Pass '{}' of feature '{}' has no valid executor: {}. '{}' is left out of the render plan.",
						                passName, featureName, executeFn.error().Message, entry.Name);
						return false;
					}

					resolved.ExecuteFn = std::move(*executeFn);
				}

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

	void RenderPlanBuilder::ResolveBindlessBindings(std::vector<ResolvedPass>& passes) const
	{
		for (ResolvedPass& pass : passes)
		{
			if (pass.Shaders.empty())
				continue;

			// The stages of a pass share one push constant block, so a field must sit at the same place in all of them
			std::vector<ShaderBindlessField> fields;
			for (const auto& [shaderPath, shaderStage] : pass.Shaders)
			{
				const PolyID      shaderID   = ShaderManager::CreateShader(shaderPath, shaderStage);
				const ShaderData& shaderData = ShaderManager::GetShader(shaderID);

				for (const ShaderBindlessField& stageField : shaderData.Reflection.BindlessFields)
				{
					auto it = std::ranges::find(fields, stageField.Name, &ShaderBindlessField::Name);
					if (it == fields.end())
						fields.push_back(stageField);
					else if (it->Offset != stageField.Offset || it->Type != stageField.Type)
						POLY_CORE_ERROR("Pass '{}' shader stages disagree on the push constant field '{}' (offset {} vs {}).",
						                pass.Name, stageField.Name, it->Offset, stageField.Offset);
				}

				for (const auto& pushConstant : shaderData.Reflection.PushConstants)
					pass.PushConstantSize = std::max(pass.PushConstantSize, pushConstant.Offset + pushConstant.Size);
			}

			if (fields.empty())
				continue;

			std::vector<bool> isBound(fields.size(), false);

			for (uint32 portIndex = 0; portIndex < pass.Ports.size(); portIndex++)
			{
				const ResolvedPort& port      = pass.Ports[portIndex];
				const bool          isTexture = IsTextureResourceType(port.ResourceType);

				// Read/WriteResource ports have no shader variable, and attachments are not read through the push constants
				if (port.ShaderName.empty() || (!isTexture && !IsBufferResourceType(port.ResourceType)))
					continue;

				auto it = std::ranges::find(fields, port.ShaderName, &ShaderBindlessField::Name);
				if (it == fields.end())
				{
					POLY_CORE_ERROR("Pass '{}' maps '{}' to the shader name '{}', but its shaders have no push constant field with that name; it is not bound.",
					                pass.Name, port.Name, port.ShaderName);
					continue;
				}

				if (isTexture != (it->Type == EBindlessFieldType::Texture))
				{
					POLY_CORE_ERROR("Pass '{}' maps the {} '{}' to the push constant field '{}', which is a {}; it is not bound.",
					                pass.Name, isTexture ? "texture" : "buffer", port.Name, port.ShaderName, isTexture ? "buffer pointer" : "TextureHandle");
					continue;
				}

				isBound[it - fields.begin()] = true;

				if (isTexture)
					pass.TextureBindings.push_back({it->Offset, portIndex});
				else
					pass.BufferBindings.push_back({it->Offset, portIndex});
			}

			for (size_t i = 0; i < fields.size(); i++)
			{
				// A texture may be left to the execute function (ExecuteContext::SetTexture), a buffer pointer may not
				if (fields[i].Type == EBindlessFieldType::Texture)
					pass.TextureFields.push_back({fields[i].Name, fields[i].Offset});
				else if (!isBound[i])
					POLY_CORE_ERROR("Pass '{}' has the buffer pointer '{}' in its push constants, but no resource is mapped to that shader name; it is left as null.",
					                pass.Name, fields[i].Name);
			}
		}
	}
} // namespace Poly
