// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "XInputHandler.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

XInputHandler::XInputHandler()
{
	ZeroMemory(&m_gamepadState, sizeof(XINPUT_STATE));

	// Simply get the state of the controller from XInput.
	XInputGetState(0, &m_gamepadState);
}

void XInputHandler::Update()
{
	Update_GamepadState();
}

void XInputHandler::Update_GamepadState()
{
	XINPUT_STATE newState;
	ZeroMemory(&newState, sizeof(XINPUT_STATE));

	XInputGetState(0, &newState);

	Update_GamepadState_UpdateButtonsState(newState);
	Update_GamepadState_UpdateAxesState(newState);
	Update_GamepadState_UpdateTriggersState(newState);
	
	m_gamepadState = newState;
}

void XInputHandler::Update_GamepadState_UpdateButtonsState(XINPUT_STATE newState)
{
	for (auto buttonCode : s_gamepadButtonCodes)
	{
		const GamepadButtonState status = CheckGamepadButtonState(m_gamepadState, newState, buttonCode);
		if (status == GamepadButtonState::Pressed)
		{
			m_onGamepadButtonPressed.Dispatch(buttonCode);
		}
		else if (status == GamepadButtonState::Released)
		{
			m_onGamepadButtonReleased.Dispatch(buttonCode);
		}
	}
}

void XInputHandler::Update_GamepadState_UpdateAxesState(XINPUT_STATE newState)
{
	const vector leftStickInput = { static_cast<float>(newState.Gamepad.sThumbLX), static_cast<float>(newState.Gamepad.sThumbLY), 0.f };
	NormalizedGamepadStickInput normalizedLeftStickInput = CalculateNormalizedGamepadStickInput(leftStickInput, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, s_gamepadStickMaxValue);

	const vector rightStickInput = { static_cast<float>(newState.Gamepad.sThumbRX), static_cast<float>(newState.Gamepad.sThumbRY), 0.f };
	NormalizedGamepadStickInput normalizedRightStickInput = CalculateNormalizedGamepadStickInput(rightStickInput, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, s_gamepadStickMaxValue);
	// send events
}

void XInputHandler::Update_GamepadState_UpdateTriggersState(XINPUT_STATE newState)
{
	const float normalizedLeftTrigger = static_cast<float>(newState.Gamepad.bLeftTrigger) / s_triggerCap;
	const float normalizedRightTrigger = static_cast<float>(newState.Gamepad.bRightTrigger) / s_triggerCap;
	// send events
}

XInputHandler::GamepadButtonState XInputHandler::CheckGamepadButtonState(const XINPUT_STATE& prevGamepadState, const XINPUT_STATE& currentGamepadState, uint64_t buttonCode)
{
	const bool wasPressed = prevGamepadState.Gamepad.wButtons & buttonCode;
	const bool isPressed = currentGamepadState.Gamepad.wButtons & buttonCode;

	if (!wasPressed && isPressed) { return GamepadButtonState::Pressed; }
	else if (wasPressed && !isPressed) { return GamepadButtonState::Released; }
	else { return GamepadButtonState::Unchanged; }
}

XInputHandler::NormalizedGamepadStickInput XInputHandler::CalculateNormalizedGamepadStickInput(const vector& stickInput, float deadzone, float maxValue)
{
	// determine how far the controller is pushed
	float magnitude = stickInput.length();
	vector normalizedInput = stickInput.getNormalized();

	float normalizedMagnitude = 0;

	// check if the controller is outside a circular dead zone
	if (magnitude > deadzone)
	{
		// clip the magnitude at its expected maximum value
		// TODO add clamp function
		if (magnitude > maxValue)
		{
			magnitude = maxValue;
		}

		// adjust magnitude relative to the end of the dead zone
		magnitude -= deadzone;

		// optionally normalize the magnitude with respect to its expected range
		// giving a magnitude value of 0.0 to 1.0
		normalizedMagnitude = magnitude / (maxValue - deadzone);
	}
	else // if the controller is in the deadzone zero out the magnitude
	{
		normalizedInput = vector::zero;
		magnitude = 0.0;
		normalizedMagnitude = 0.0;
	}

	return NormalizedGamepadStickInput{ normalizedInput, normalizedMagnitude };
}

}
#endif
}
