#include "HierarchyPanel.h"

#include <imgui/imgui.h>

namespace
{
	constexpr const char* ENTITY_PAYLOAD = "POLY_ENTITY";

	void ShowError(const Poly::Result<void>& result)
	{
		if (!result)
			POLY_WARN("{}", result.error().Message);
	}
} // namespace

namespace Editor
{
	void HierarchyPanel::Draw(Poly::API::EngineContext& context, EditorState& state)
	{
		m_Action = EAction::None;

		if (ImGui::Begin("Hierarchy"))
		{
			for (Poly::PolyID root : context.Entities().GetRoots())
				DrawEntity(context, state, root);

			// Empty space below the tree: click to deselect, drop to unparent, right click to create a root entity
			ImGui::InvisibleButton("##Background", ImVec2(std::max(ImGui::GetContentRegionAvail().x, 1.0f), std::max(ImGui::GetContentRegionAvail().y, 50.0f)));
			if (ImGui::IsItemClicked())
				state.Selection = Poly::PolyID::None();

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(ENTITY_PAYLOAD))
				{
					m_Action = EAction::SetParent;
					m_Target = Poly::PolyID(*static_cast<const uint64*>(pPayload->Data));
					m_Parent = Poly::PolyID::None();
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::BeginPopupContextItem("##BackgroundMenu"))
			{
				if (ImGui::MenuItem("Create Entity"))
				{
					m_Action = EAction::Create;
					m_Parent = Poly::PolyID::None();
				}
				ImGui::EndPopup();
			}
		}
		ImGui::End();

		switch (m_Action)
		{
		case EAction::Create:
			if (Poly::Result<Poly::PolyID> created = context.Entities().Create(m_Parent))
				state.Selection = *created;
			else
				POLY_WARN("{}", created.error().Message);
			break;
		case EAction::Destroy:
			ShowError(context.Entities().Destroy(m_Target));
			break;
		case EAction::SetParent:
			ShowError(context.Entities().SetParent(m_Target, m_Parent));
			break;
		case EAction::None:
			break;
		}
	}

	void HierarchyPanel::DrawEntity(Poly::API::EngineContext& context, EditorState& state, Poly::PolyID id)
	{
		const Poly::Result<Poly::API::EntityInfo> info = context.Entities().Get(id);
		if (!info)
			return;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (info->Children.empty())
			flags |= ImGuiTreeNodeFlags_Leaf;
		if (state.Selection == id)
			flags |= ImGuiTreeNodeFlags_Selected;

		const uint64      rawID = static_cast<uint64>(id);
		const std::string label = info->Name.empty() ? "Entity" : info->Name;

		ImGui::PushID(reinterpret_cast<const void*>(rawID));
		const bool open = ImGui::TreeNodeEx("##Node", flags, "%s", label.c_str());

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			state.Selection = id;

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(ENTITY_PAYLOAD, &rawID, sizeof(rawID));
			ImGui::TextUnformatted(label.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(ENTITY_PAYLOAD))
			{
				m_Action = EAction::SetParent;
				m_Target = Poly::PolyID(*static_cast<const uint64*>(pPayload->Data));
				m_Parent = id;
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::BeginPopupContextItem("##NodeMenu"))
		{
			if (ImGui::MenuItem("Create Child"))
			{
				m_Action = EAction::Create;
				m_Parent = id;
			}
			if (ImGui::MenuItem("Unparent", nullptr, false, info->Parent != Poly::PolyID::None()))
			{
				m_Action = EAction::SetParent;
				m_Target = id;
				m_Parent = Poly::PolyID::None();
			}
			if (ImGui::MenuItem("Destroy"))
			{
				m_Action = EAction::Destroy;
				m_Target = id;
			}
			ImGui::EndPopup();
		}

		if (open)
		{
			for (Poly::PolyID child : info->Children)
				DrawEntity(context, state, child);

			ImGui::TreePop();
		}

		ImGui::PopID();
	}
} // namespace Editor
