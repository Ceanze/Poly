#pragma once

#include "FieldVisibility.h"
#include "PassField.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <span>

namespace Poly
{
	class Sampler;
	class ShaderData;

	class PassReflection
	{
	public:
		PassReflection() = default;

		PassField& AddInput(std::string name, uint32 set, uint32 binding);
		PassField& AddOutput(std::string name);
		PassField& AddPassthrough(std::string name);

		/*
		* Adds a shader to the pass reflection. The shader reflection data will be used to populate the pass reflection.
		* (TODO) Conflicting data will try to merge is possible, otherwise be ignored and warned about.
		* @param shaderID - PolyID of an existing shader
		*/
		void AddShader(PolyID shaderID);

		bool HasField(std::string_view fieldName) const;
		const PassField& GetField(std::string_view fieldName) const;
		auto GetFields(FFieldVisibility visibility) const;
		auto GetFieldsFiltered(FFieldVisibility visibility, FResourceBindPoint excluded) const;

		void AddPushConstant(std::string name, FShaderStage shaderStage, uint64 size, uint64 offset);

		bool HasPushConstant(std::string_view name) const;
		const PushConstantData& GetPushConstant(std::string_view name) const;
		std::span<const PushConstantData> GetPushConstants() const;

		void DisableAutoBindForSet(uint32 setIndex);
		auto GetAutobindedSets() const;
		auto GetNonAutobindedSets() const;

	private:
		PassField& AddField(std::string name, FFieldVisibility visibility);
		void AddShaderBindings(const ShaderData& shader);
		void AddShaderPushConstants(const ShaderData& shader);
		void AddShaderInputs(const ShaderData& shader);
		void AddShaderOutputs(const ShaderData& shader);

		std::vector<PassField> m_Fields;
		std::vector<PushConstantData> m_PushConstants;
		std::set<uint32> m_AutoBindedSets;
		std::set<uint32> m_ManualSets;
	};
}