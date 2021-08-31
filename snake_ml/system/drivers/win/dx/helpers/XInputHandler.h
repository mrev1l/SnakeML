#pragma once
#include "lib_includes/directX_includes.h"
#include "utils/patterns/event.h"

namespace snakeml
{

#ifdef _WINDOWS
namespace win
{

class XInputHandler
{
public:
	XInputHandler();
	~XInputHandler() = default;

	void Update();

	Event<XInputHandler, uint64_t> m_onGamepadButtonPressed;
	Event<XInputHandler, uint64_t> m_onGamepadButtonReleased;

private:
	enum class GamepadButtonState
	{
		Pressed,
		Released,
		Unchanged,
	};

	struct NormalizedGamepadStickInput
	{
		vector direction = vector::zero;
		float magnitude = 0.f;
	};

	void Update_GamepadState();
	void Update_GamepadState_UpdateButtonsState(XINPUT_STATE newState);
	void Update_GamepadState_UpdateAxesState(XINPUT_STATE newState);
	void Update_GamepadState_UpdateTriggersState(XINPUT_STATE newState);

	static GamepadButtonState CheckGamepadButtonState(const XINPUT_STATE& prevGamepadState, const XINPUT_STATE& currentGamepadState, uint64_t buttonCode);
	static NormalizedGamepadStickInput CalculateNormalizedGamepadStickInput(const vector& stickInput, float deadzone, float maxValue);

	XINPUT_STATE m_gamepadState;

	static constexpr float s_triggerCap = 255.f;
	static constexpr float s_gamepadStickMaxValue = 32767.f;
	static constexpr std::array<uint64_t, 14> s_gamepadButtonCodes =
	{
		XINPUT_GAMEPAD_DPAD_UP,
		XINPUT_GAMEPAD_DPAD_DOWN,
		XINPUT_GAMEPAD_DPAD_LEFT,
		XINPUT_GAMEPAD_DPAD_RIGHT,
		XINPUT_GAMEPAD_START,
		XINPUT_GAMEPAD_BACK,
		XINPUT_GAMEPAD_LEFT_THUMB,
		XINPUT_GAMEPAD_RIGHT_THUMB,
		XINPUT_GAMEPAD_LEFT_SHOULDER,
		XINPUT_GAMEPAD_RIGHT_SHOULDER,
		XINPUT_GAMEPAD_A,
		XINPUT_GAMEPAD_B,
		XINPUT_GAMEPAD_X,
		XINPUT_GAMEPAD_Y
	};
};

}
#endif

}