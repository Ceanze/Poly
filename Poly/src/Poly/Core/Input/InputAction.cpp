#include "InputAction.h"

namespace Poly
{
	InputAction::InputAction(std::string name, EActionType type)
		: m_Name(std::move(name))
		, m_Type(type)
		, m_ActionPhase(EActionPhase::UNKNOWN) {}

	const std::string& InputAction::GetName() const
	{
		return m_Name;
	}

	EActionType InputAction::GetActionType() const
	{
		return m_Type;
	}

	void InputAction::BindButton(EKey key)
	{
		if (!ValidateType(EActionType::BUTTON))
			return;

		m_Inputs[0] = ActionAxis(key, key);
	}

	void InputAction::BindAxis1D(ActionAxis axis)
	{
		if (!ValidateType(EActionType::AXIS1D))
			return;

		m_Inputs[0] = axis;
	}

	void InputAction::BindAxis2D(ActionAxis axisLeftRight, ActionAxis axisUpDown)
	{
		if (!ValidateType(EActionType::AXIS2D))
			return;

		m_Inputs[0] = axisLeftRight;
		m_Inputs[1] = axisUpDown;
	}

	void InputAction::BindAxis3D(ActionAxis axisLeftRight, ActionAxis axisUpDown, ActionAxis axisInOut)
	{
		if (!ValidateType(EActionType::AXIS3D))
			return;

		m_Inputs[0] = axisLeftRight;
		m_Inputs[1] = axisUpDown;
		m_Inputs[2] = axisInOut;
	}

	void InputAction::ButtonCallback(ButtonCB callback)
	{
		if (!ValidateType(EActionType::BUTTON))
			return;

		m_Callback = std::move(callback);
	}

	void InputAction::Axis1DCallback(Axis1DCB callback)
	{
		if (!ValidateType(EActionType::AXIS1D))
			return;

		m_Callback = std::move(callback);
	}

	void InputAction::Axis2DCallback(Axis2DCB callback)
	{
		if (!ValidateType(EActionType::AXIS2D))
			return;

		m_Callback = std::move(callback);
	}

	void InputAction::Axis3DCallback(Axis3DCB callback)
	{
		if (!ValidateType(EActionType::AXIS3D))
			return;

		m_Callback = std::move(callback);
	}

	bool InputAction::IsStarted() const
	{
		if (!ValidateType(EActionType::BUTTON))
			return false;

		return m_ActionPhase == EActionPhase::STARTED;
	}

	bool InputAction::IsPerforming() const
	{
		if (!ValidateType(EActionType::BUTTON))
			return false;

		return m_ActionPhase == EActionPhase::PERFORMING;
	}

	bool InputAction::IsStopped() const
	{
		if (!ValidateType(EActionType::BUTTON))
			return false;

		return m_ActionPhase == EActionPhase::STOPPED;
	}

	float InputAction::GetAxis1D() const
	{
		if (!ValidateType(EActionType::AXIS1D))
			return 0.0f;

		return m_Values.x;
	}

	glm::vec2 InputAction::GetAxis2D() const
	{
		if (!ValidateType(EActionType::AXIS2D))
			return {};

		return glm::vec2(m_Values);
	}

	glm::vec3 InputAction::GetAxis3D() const
	{
		if (!ValidateType(EActionType::AXIS3D))
			return {};

		return m_Values;
	}

	void InputAction::SetButton(EActionPhase actionPhase)
	{
		if (!ValidateType(EActionType::BUTTON))
			return;

		m_ActionPhase = actionPhase;

		if (const auto callback = std::get<ButtonCB>(m_Callback))
			callback(actionPhase);
	}

	void InputAction::SetAxis1D(float value)
	{
		if (!ValidateType(EActionType::AXIS1D))
			return;

		m_Values.x = value;

		if (const auto callback = std::get<Axis1DCB>(m_Callback))
			callback(value);
	}

	void InputAction::SetAxis2D(float x, float y)
	{
		if (!ValidateType(EActionType::AXIS2D))
			return;

		m_Values.x = x;
		m_Values.y = y;

		if (const auto callback = std::get<Axis2DCB>(m_Callback))
			callback(x, y);
	}

	void InputAction::SetAxis3D(float x, float y, float z)
	{
		if (!ValidateType(EActionType::AXIS3D))
			return;

		m_Values.x = x;
		m_Values.y = y;
		m_Values.z = z;

		if (const auto callback = std::get<Axis3DCB>(m_Callback))
			callback(x, y, z);
	}

	bool InputAction::ValidateType(EActionType functionType) const
	{
		if (functionType != m_Type)
		{
			const auto toString = [](EActionType type) {
				switch (type) {
				case EActionType::BUTTON:
					return "EActionType::BUTTON";
				case EActionType::AXIS1D:
					return "EActionType::AXIS1D";
				case EActionType::AXIS2D:
					return "EActionType::AXIS2D";
				case EActionType::AXIS3D:
					return "EActionType::AXIS3D";
				}
			};

			POLY_CORE_WARN("Invalid action type {} used for a function with an action type of {}", toString(m_Type), toString(functionType));
			return false;
		}

		return true;
	}
}