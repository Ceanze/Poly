#include "ImGuiFieldVisitor.h"

#include <imgui/imgui.h>

namespace
{
	constexpr float DRAG_SPEED = 0.05f;

	// Null terminated copy, the names of fields are views
	std::string Label(std::string_view name)
	{
		return std::string(name);
	}

	bool InputString(const std::string& label, std::string& value, ImGuiInputTextFlags flags = 0)
	{
		char buffer[512] = {};
		value.copy(buffer, sizeof(buffer) - 1);
		if (!ImGui::InputText(label.c_str(), buffer, sizeof(buffer), flags))
			return false;

		value = buffer;
		return true;
	}
} // namespace

namespace Editor
{
	ImGuiFieldVisitor::ImGuiFieldVisitor(Poly::API::CommandHistory& history)
	    : m_History(history)
	{}

	bool ImGuiFieldVisitor::Visit(std::string_view name, bool& value)
	{
		return ImGui::Checkbox(Label(name).c_str(), &value);
	}

	bool ImGuiFieldVisitor::Visit(std::string_view name, int32& value)
	{
		const bool changed = ImGui::DragScalar(Label(name).c_str(), ImGuiDataType_S32, &value);
		TrackMerge();
		return changed;
	}

	bool ImGuiFieldVisitor::Visit(std::string_view name, uint32& value)
	{
		const bool changed = ImGui::DragScalar(Label(name).c_str(), ImGuiDataType_U32, &value);
		TrackMerge();
		return changed;
	}

	bool ImGuiFieldVisitor::Visit(std::string_view name, float& value)
	{
		const bool changed = ImGui::DragFloat(Label(name).c_str(), &value, DRAG_SPEED);
		TrackMerge();
		return changed;
	}

	bool ImGuiFieldVisitor::Visit(std::string_view name, glm::vec2& value)
	{
		const bool changed = ImGui::DragFloat2(Label(name).c_str(), &value.x, DRAG_SPEED);
		TrackMerge();
		return changed;
	}

	bool ImGuiFieldVisitor::Visit(std::string_view name, glm::vec3& value)
	{
		const bool changed = ImGui::DragFloat3(Label(name).c_str(), &value.x, DRAG_SPEED);
		TrackMerge();
		return changed;
	}

	bool ImGuiFieldVisitor::Visit(std::string_view name, glm::vec4& value)
	{
		const bool changed = ImGui::DragFloat4(Label(name).c_str(), &value.x, DRAG_SPEED);
		TrackMerge();
		return changed;
	}

	bool ImGuiFieldVisitor::Visit(std::string_view name, glm::quat& value)
	{
		// TODO: Keep the euler angles while editing, converting back and forth every frame makes them jump around the poles
		glm::vec3  euler   = glm::degrees(glm::eulerAngles(value));
		const bool changed = ImGui::DragFloat3(Label(name).c_str(), &euler.x, 0.5f);
		TrackMerge();
		if (changed)
			value = glm::quat(glm::radians(euler));

		return changed;
	}

	bool ImGuiFieldVisitor::Visit(std::string_view name, std::string& value)
	{
		const bool changed = InputString(Label(name), value);
		TrackMerge();
		return changed;
	}

	bool ImGuiFieldVisitor::VisitAsset(std::string_view name, std::string& path)
	{
		// Setting the field loads the asset, so only when the whole path has been entered
		return InputString(Label(name), path, ImGuiInputTextFlags_EnterReturnsTrue);
	}

	void ImGuiFieldVisitor::EndMergeIfDone()
	{
		if (m_Deactivated)
			m_History.EndMerge();

		m_Deactivated = false;
	}

	void ImGuiFieldVisitor::TrackMerge()
	{
		if (ImGui::IsItemActivated())
			m_History.BeginMerge();

		if (ImGui::IsItemDeactivated())
			m_Deactivated = true;
	}
} // namespace Editor
