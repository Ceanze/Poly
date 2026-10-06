#pragma once

#include <Poly/API/CommandHistory.h>
#include <Poly/Reflection/FieldVisitor.h>

namespace Editor
{
	/*
	 * Draws an ImGui widget per visited field.
	 *
	 * Dragging a widget changes the field every frame, the changes made while a widget is active are merged into a
	 * single undo step: the merge is started as soon as a widget is activated, and EndMergeIfDone() must be called
	 * once the changes of the frame have been applied to end it when the widget was let go.
	 */
	class ImGuiFieldVisitor : public Poly::FieldVisitor
	{
	public:
		explicit ImGuiFieldVisitor(Poly::API::CommandHistory& history);

		bool Visit(std::string_view name, bool& value) override;
		bool Visit(std::string_view name, int32& value) override;
		bool Visit(std::string_view name, uint32& value) override;
		bool Visit(std::string_view name, float& value) override;
		bool Visit(std::string_view name, glm::vec2& value) override;
		bool Visit(std::string_view name, glm::vec3& value) override;
		bool Visit(std::string_view name, glm::vec4& value) override;
		bool Visit(std::string_view name, glm::quat& value) override;
		bool Visit(std::string_view name, std::string& value) override;
		bool VisitAsset(std::string_view name, std::string& path) override;

		void EndMergeIfDone();

	private:
		// Call right after drawing a widget
		void TrackMerge();

		Poly::API::CommandHistory& m_History;
		bool                       m_Deactivated = false;
	};
} // namespace Editor
