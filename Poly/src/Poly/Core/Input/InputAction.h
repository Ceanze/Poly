#pragma once

#include "ActionType.h"
#include "ActionAxis.h"
#include "ActionPhase.h"
#include "Keys.h"

#include <variant>

namespace Poly
{
	class InputAction
	{
	public:
		using ButtonCB = std::function<void(EActionPhase)>;
		using Axis1DCB = std::function<void(float)>;
		using Axis2DCB = std::function<void(float, float)>;
		using Axis3DCB = std::function<void(float, float, float)>;

	public:
		InputAction(std::string name, EActionType type);

		const std::string& GetName() const;
		EActionType GetActionType() const;

		void BindButton(EKey key);
		void BindAxis1D(ActionAxis axis);
		void BindAxis2D(ActionAxis axisLeftRight, ActionAxis axisUpDown);
		void BindAxis3D(ActionAxis axisLeftRight, ActionAxis axisUpDown, ActionAxis axisInOut);

		void ButtonCallback(ButtonCB callback);
		void Axis1DCallback(Axis1DCB callback);
		void Axis2DCallback(Axis2DCB callback);
		void Axis3DCallback(Axis3DCB callback);

		bool IsStarted() const;
		bool IsPerforming() const;
		bool IsStopped() const;

		float GetAxis1D() const;
		glm::vec2 GetAxis2D() const;
		glm::vec3 GetAxis3D() const;

		void SetButton(EActionPhase actionPhase);
		void SetAxis1D(float value);
		void SetAxis2D(float x, float y);
		void SetAxis3D(float x, float y, float z);

	private:
		bool ValidateType(EActionType functionType) const;

		std::string m_Name;
		EActionType m_Type;

		EActionPhase m_ActionPhase;
		glm::vec3 m_Values;
		std::array<ActionAxis, 3> m_Inputs;
		std::variant<ButtonCB, Axis1DCB, Axis2DCB, Axis3DCB> m_Callback;
	};
}