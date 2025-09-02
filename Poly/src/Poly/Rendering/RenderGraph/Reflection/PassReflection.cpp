#include "PassReflection.h"

#include "Poly/Resources/Shader/ShaderManager.h"

#include <ranges>

namespace Poly
{
	PassField& PassReflection::AddInput(std::string name, uint32 set, uint32 binding)
	{
		PassField& field = AddField(std::move(name), FFieldVisibility::INPUT);

		if (!m_ManualSets.contains(set))
			m_AutoBindedSets.insert(set);

		return field.Set(set).Binding(binding);
	}

	PassField& PassReflection::AddOutput(std::string name)
	{
		return AddField(std::move(name), FFieldVisibility::OUTPUT);
	}

	PassField& PassReflection::AddPassthrough(std::string name)
	{
		return AddField(std::move(name), FFieldVisibility::IN_OUT);
	}

	void PassReflection::AddShader(PolyID shaderID)
	{
		if (!ShaderManager::ShaderExists(shaderID))
		{
			POLY_CORE_WARN("Shader {} added to Pass Reflection has not been created or is invalid", shaderID);
			return;
		}

		const ShaderData& shader = ShaderManager::GetShader(shaderID);
		
		AddShaderBindings(shader);
		AddShaderInputs(shader);
		AddShaderPushConstants(shader);

		// Only outputs from fragment shader is currently out of interest
		if (BitsSet(shader.ShaderStage, FShaderStage::FRAGMENT))
			AddShaderOutputs(shader);
	}

	bool PassReflection::HasField(std::string_view fieldName) const
	{
		auto it = std::find_if(m_Fields.begin(), m_Fields.end(), [fieldName](const PassField& field) { return field.GetName() == fieldName; });

		return it != m_Fields.end();
	}

	PassField& PassReflection::GetField(std::string_view fieldName)
	{
		auto it = std::find_if(m_Fields.begin(), m_Fields.end(), [fieldName](const PassField& field) { return field.GetName() == fieldName; });
		POLY_VALIDATE(it != m_Fields.end(), "Reflection with field name {} could not be found", fieldName);

		return *it;
	}

	const PassField& PassReflection::GetField(std::string_view fieldName) const
	{
		const auto it = std::find_if(m_Fields.begin(), m_Fields.end(), [fieldName](const PassField& field) { return field.GetName() == fieldName; });
		POLY_VALIDATE(it != m_Fields.end(), "Reflection with field name {} could not be found", fieldName);

		return *it;
	}

	std::vector<PassField*> PassReflection::GetFields(FFieldVisibility visibility)
	{
		std::vector<PassField*> result;

		for (auto& field : m_Fields)
		{
			if (BitsSet(field.GetVisibility(), visibility))
				result.push_back(&field);
		}

		return result;
	}

	std::vector<const PassField*> PassReflection::GetFields(FFieldVisibility visibility) const
	{
		std::vector<const PassField*> result;

		for (auto& field : m_Fields)
		{
			if (BitsSet(field.GetVisibility(), visibility))
				result.push_back(&field);
		}

		return result;
	}

	std::vector<PassField*> PassReflection::GetFieldsFiltered(FFieldVisibility visibility, FResourceBindPoint exclusion)
	{
		std::vector<PassField*> result;

		for (auto& field : m_Fields)
		{
			if (BitsSet(field.GetVisibility(), visibility) && !BitsSet(field.GetBindPoint(), exclusion))
				result.push_back(&field);
		}

		return result;
	}

	std::vector<const PassField*> PassReflection::GetFieldsFiltered(FFieldVisibility visibility, FResourceBindPoint exclusion) const
	{
		std::vector<const PassField*> result;

		for (auto& field : m_Fields)
		{
			if (BitsSet(field.GetVisibility(), visibility) && !BitsSet(field.GetBindPoint(), exclusion))
				result.push_back(&field);
		}

		return result;
	}

	void PassReflection::AddPushConstant(std::string name, FShaderStage shaderStage, uint64 size, uint64 offset)
	{
		// TODO: Check push constant validation - shader stage could play a role
		// If above is true, then the check should check overlaps, based on offset+size, names, but only for the same shader stages
		const auto itr = std::ranges::find_if(m_PushConstants, [&name, &offset](const PushConstantData& data) { return data.Name == name || data.Offset == offset; });
		if (itr != m_PushConstants.end())
		{
			POLY_CORE_WARN("PushConstant '{}' has already been added with the same name or offset of {}", name, offset);
			return;
		}

		m_PushConstants.push_back({ std::move(name), size, offset, shaderStage });
	}

	bool PassReflection::HasPushConstant(std::string_view name) const
	{
		return std::ranges::find_if(m_PushConstants, [&name](const PushConstantData& data) { return data.Name == name; }) != m_PushConstants.end();
	}

	const PushConstantData& PassReflection::GetPushConstant(std::string_view name) const
	{
		const auto itr = std::ranges::find_if(m_PushConstants, [&name](const PushConstantData& data) { return data.Name == name; });
		POLY_VALIDATE(itr != m_PushConstants.end(), "Reflection with push constant name {} could not be found", name);

		return *itr;
	}

	std::span<const PushConstantData> PassReflection::GetPushConstants() const
	{
		return { m_PushConstants.data(), m_PushConstants.size() };
	}

	void PassReflection::DisableAutoBindForSet(uint32 setIndex)
	{
		m_AutoBindedSets.erase(setIndex);
		m_ManualSets.insert(setIndex);
	}

	const std::set<uint32>& PassReflection::GetAutoBindedSets() const
	{
		return m_AutoBindedSets;
	}

	const std::set<uint32>& PassReflection::GetNonAutoBindedSets() const
	{
		return m_ManualSets;
	}

	void PassReflection::PrintDebug() const
	{
		for (const auto& field : m_Fields)
		{
			std::string visStr;
			FFieldVisibility vis = field.GetVisibility();
			if (BitsSet(vis, FFieldVisibility::INPUT))
				visStr = "INPUT";
			else if (BitsSet(vis, FFieldVisibility::OUTPUT))
				visStr = "OUTPUT";
			else if (BitsSet(vis, FFieldVisibility::IN_OUT))
				visStr = "PASSTHROUGH";

			POLY_CORE_TRACE("Field: {}", field.GetName());
			POLY_CORE_TRACE("  Set {}", field.GetSet());
			POLY_CORE_TRACE("  Binding {}", field.GetBinding());
			POLY_CORE_TRACE("  BindPoint {}", static_cast<int>(field.GetBindPoint()));
			POLY_CORE_TRACE("  Size {}", field.GetSize());
			POLY_CORE_TRACE("  Width {}", field.GetWidth());
			POLY_CORE_TRACE("  Height {}", field.GetHeight());
			POLY_CORE_TRACE("  Depth {}", field.GetDepth());
			POLY_CORE_TRACE("  Format {}", static_cast<int>(field.GetFormat()));
			POLY_CORE_TRACE("  Texture Layout {}", static_cast<int>(field.GetTextureLayout()));
			POLY_CORE_TRACE("  Sampler? {}", field.GetSampler().get() != nullptr);
			POLY_CORE_TRACE("  Visibility {}", visStr);
		}
	}

	PassField& PassReflection::AddField(std::string name, FFieldVisibility visibility)
	{
		// If field exist, merge visibilities - if not possible, error
		auto itr = std::ranges::find_if(m_Fields, [name](const PassField& field) { return field.GetName() == name; });
		if (itr != m_Fields.end())
		{
			const FFieldVisibility existingVis = itr->GetVisibility();
			bool validExistingVisibility = BitsSet(existingVis, FFieldVisibility::IN_OUT);
			bool validRequestedVisibility = BitsSet(visibility, FFieldVisibility::IN_OUT);

			if (validExistingVisibility && validRequestedVisibility)
			{
				itr->m_Visibility = visibility | existingVis;
			}
			else
			{
				POLY_CORE_ERROR("Cannot merge field {} with already existing field, visibilites of {} and {} are not valid to merge", name, static_cast<int>(visibility), static_cast<int>(existingVis));
			}

			return *itr;
		}

		m_Fields.push_back({ name, visibility });
		return m_Fields.back();
	}

	void PassReflection::AddShaderBindings(const ShaderData& shader)
	{
		for (const auto& binding : shader.Reflection.Bindings)
		{
			AddInput(binding.Name, binding.Set, binding.Binding)
				.BindPoint(binding.DescriptorType);
		}
	}

	void PassReflection::AddShaderPushConstants(const ShaderData& shader)
	{
		for (const auto& pc : shader.Reflection.PushConstants)
		{
			AddPushConstant(pc.Name, shader.ShaderStage, pc.Size, pc.Offset);
		}
	}

	void PassReflection::AddShaderInputs(const ShaderData& shader)
	{
		// NOT SUPPORTED YET
		// TODO: Support this
	}

	void PassReflection::AddShaderOutputs(const ShaderData& shader)
	{
		for (const auto& output : shader.Reflection.Outputs)
		{
			AddField(output.Name, FFieldVisibility::OUTPUT);
		}
	}
}