#include "PassReflection.h"

#include <ranges>

namespace Poly
{
	PassField& Poly::PassReflection::AddInput(std::string name, uint32 set, uint32 binding)
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

	bool PassReflection::HasField(std::string_view fieldName) const
	{
		auto it = std::find_if(m_Fields.begin(), m_Fields.end(), [fieldName](const PassField& field) { return field.GetName() == fieldName; });

		return it != m_Fields.end();
	}

	const PassField& PassReflection::GetField(std::string_view fieldName) const
	{
		auto it = std::find_if(m_Fields.begin(), m_Fields.end(), [fieldName](const PassField& field) { return field.GetName() == fieldName; });
		POLY_VALIDATE(it != m_Fields.end(), "Reflection with field name {} could not be found", fieldName);

		return *it;
	}

	auto PassReflection::GetFields(FFieldVisibility visibility) const
	{
		return m_Fields
			| std::views::filter([&visibility](const PassField& field) {
				return field.GetVisibility() == visibility;
			});
	}

	auto PassReflection::GetFieldsFiltered(FFieldVisibility visibility, FResourceBindPoint exclusion) const
	{
		return m_Fields
			| std::views::filter([&visibility, &exclusion](const PassField& field) {
				return field.GetVisibility() == visibility && (field.GetBindPoint() & exclusion) == FResourceBindPoint::NONE;
			});
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

	auto PassReflection::GetAutobindedSets() const
	{
		return m_AutoBindedSets | std::views::all;
	}

	auto PassReflection::GetNonAutobindedSets() const
	{
		return m_ManualSets | std::views::all;
	}

	PassField& PassReflection::AddField(std::string name, FFieldVisibility visibility)
	{
		POLY_VALIDATE(!HasField(name), "Tried adding field {}, but field name already exists", name);

		m_Fields.push_back({ name, visibility });
		return m_Fields.back();
	}
}