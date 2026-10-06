#include "InspectorPanel.h"

#include "Panels/ImGuiFieldVisitor.h"

#include <imgui/imgui.h>

namespace
{
	template<typename T>
	void ShowError(const Poly::Result<T>& result)
	{
		if (!result)
			POLY_WARN("{}", result.error().Message);
	}
} // namespace

namespace Editor
{
	void InspectorPanel::Draw(Poly::API::EngineContext& context, EditorState& state)
	{
		if (ImGui::Begin("Inspector"))
		{
			const Poly::Result<Poly::API::EntityInfo> info = context.Entities().Get(state.Selection);
			if (!info)
			{
				ImGui::TextDisabled("No entity selected");
				ImGui::End();
				return;
			}

			ImGui::TextDisabled("ID %llu", static_cast<unsigned long long>(static_cast<uint64>(info->ID)));

			const std::vector<Poly::ComponentTypeInfo> types = context.Components().GetTypes();

			std::string        toRemove;
			ImGuiFieldVisitor visitor(context.History());
			for (const std::string& component : info->Components)
			{
				ImGui::PushID(component.c_str());

				if (ImGui::CollapsingHeader(component.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					// Fields get their own ID scope, a field named like its component ("Name") or like
					// a widget of the header ("Remove") would otherwise share the ID with it
					ImGui::PushID("Fields");
					ShowError(context.Components().Inspect(state.Selection, component, visitor));
					ImGui::PopID();

					auto type = std::ranges::find(types, component, &Poly::ComponentTypeInfo::Name);
					if (type != types.end() && type->Removable && ImGui::SmallButton("Remove"))
						toRemove = component;
				}

				ImGui::PopID();
			}
			visitor.EndMergeIfDone();

			if (!toRemove.empty())
				ShowError(context.Components().Remove(state.Selection, toRemove));

			ImGui::Separator();

			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("##AddComponent");

			if (ImGui::BeginPopup("##AddComponent"))
			{
				const Poly::Result<std::vector<std::string>> addable = context.Components().GetAddable(state.Selection);
				if (addable && addable->empty())
					ImGui::TextDisabled("Nothing to add");

				for (const std::string& component : addable.value_or({}))
				{
					if (ImGui::MenuItem(component.c_str()))
						ShowError(context.Components().Add(state.Selection, component));
				}
				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}
} // namespace Editor
