/*
WIIU_INPUT.C

Xbox controllers for the Wii U build (port/linux/src/xinput_sdl.c is the
Linux build's): the GamePad is the Xbox's port 1 (the game's controller 0),
and a Pro or Classic Controller on Wii remote channels 1 to 3 is port 2 to 4
(wiiu_pads.c reads them). The buttons go by where they sit, as the Xbox's
did: the bottom face button (the Wii U's B) is the Xbox's A, the right one
(the Wii U's A) is B, left (Y) is X and top (X) is Y. ZR and ZL are the
triggers (fire, grenade), R and L the black and white buttons, + and -
Start and Back. There is no keyboard.
*/

#include "platform.h"
#include "halo_keyboard.h"
#include "wiiu_pads.h"

#include <string.h>

XPP_DEVICE_TYPE XDEVICE_TYPE_GAMEPAD_TABLE;
XPP_DEVICE_TYPE XDEVICE_TYPE_MEMORY_UNIT_TABLE;
XPP_DEVICE_TYPE XDEVICE_TYPE_DEBUG_KEYBOARD_TABLE;

struct controller
{
	BOOL open;
	DWORD packet_number;
	XINPUT_GAMEPAD previous;
};

static struct controller controllers[WIIU_PAD_PORTS];
static DWORD reported_gamepads;

VOID WINAPI XInitDevices(DWORD preallocation_type_count, PXDEVICE_PREALLOC_TYPE preallocation_types)
{
	(void)preallocation_type_count;
	(void)preallocation_types;
	wiiu_pads_initialize();
}

static DWORD connected_gamepads(void)
{
	DWORD mask = 0;
	int port;

	for (port = 0; port < WIIU_PAD_PORTS; port++)
	{
		struct wiiu_pad pad;

		wiiu_pad_read(port, &pad);
		if (pad.connected)
			mask |= 1UL << port;
	}
	return mask;
}

BOOL WINAPI XGetDeviceChanges(PXPP_DEVICE_TYPE device_type, PDWORD insertions, PDWORD removals)
{
	*insertions = 0;
	*removals = 0;
	if (device_type == XDEVICE_TYPE_GAMEPAD)
	{
		DWORD connected = connected_gamepads();

		*insertions = connected & ~reported_gamepads;
		*removals = reported_gamepads & ~connected;
		reported_gamepads = connected;
	}
	return *insertions || *removals;
}

HANDLE WINAPI XInputOpen(PXPP_DEVICE_TYPE device_type, DWORD port, DWORD slot,
	PXINPUT_POLLING_PARAMETERS polling_parameters)
{
	(void)slot;
	(void)polling_parameters;
	if (device_type == XDEVICE_TYPE_GAMEPAD && port < WIIU_PAD_PORTS)
	{
		memset(&controllers[port], 0, sizeof(controllers[port]));
		controllers[port].open = TRUE;
		return (HANDLE)&controllers[port];
	}
	SetLastError(ERROR_DEVICE_NOT_CONNECTED);
	return NULL;
}

VOID WINAPI XInputClose(HANDLE device)
{
	struct controller *controller = (struct controller *)device;

	if (controller)
		controller->open = FALSE;
}

static int controller_port(HANDLE device)
{
	int port;

	for (port = 0; port < WIIU_PAD_PORTS; port++)
	{
		if (device == (HANDLE)&controllers[port] && controllers[port].open)
			return port;
	}
	return -1;
}

/* a stick's -1 to 1 as the Xbox's -32768 to 32767 */
static SHORT stick(float value)
{
	float scaled = value * 32767.0f;

	if (scaled < -32768.0f) scaled = -32768.0f;
	if (scaled > 32767.0f) scaled = 32767.0f;
	return (SHORT)scaled;
}

static void pad_state(const struct wiiu_pad *pad, XINPUT_GAMEPAD *gamepad)
{
	static const struct
	{
		unsigned int button;
		WORD mask;
	} digital[] =
	{
		{ WIIU_PAD_UP, XINPUT_GAMEPAD_DPAD_UP },
		{ WIIU_PAD_DOWN, XINPUT_GAMEPAD_DPAD_DOWN },
		{ WIIU_PAD_LEFT, XINPUT_GAMEPAD_DPAD_LEFT },
		{ WIIU_PAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT },
		{ WIIU_PAD_PLUS, XINPUT_GAMEPAD_START },
		{ WIIU_PAD_MINUS, XINPUT_GAMEPAD_BACK },
		{ WIIU_PAD_STICK_L, XINPUT_GAMEPAD_LEFT_THUMB },
		{ WIIU_PAD_STICK_R, XINPUT_GAMEPAD_RIGHT_THUMB },
	};
	/* the Xbox's analog buttons; the Wii U's are all fully in or out */
	static const struct
	{
		unsigned int button;
		int index;
	} analog[] =
	{
		{ WIIU_PAD_SOUTH, XINPUT_GAMEPAD_A },
		{ WIIU_PAD_EAST, XINPUT_GAMEPAD_B },
		{ WIIU_PAD_WEST, XINPUT_GAMEPAD_X },
		{ WIIU_PAD_NORTH, XINPUT_GAMEPAD_Y },
		{ WIIU_PAD_L, XINPUT_GAMEPAD_WHITE },
		{ WIIU_PAD_R, XINPUT_GAMEPAD_BLACK },
		{ WIIU_PAD_ZL, XINPUT_GAMEPAD_LEFT_TRIGGER },
		{ WIIU_PAD_ZR, XINPUT_GAMEPAD_RIGHT_TRIGGER },
	};
	unsigned int index;

	for (index = 0; index < sizeof(digital) / sizeof(digital[0]); index++)
	{
		if (pad->buttons & digital[index].button)
			gamepad->wButtons |= digital[index].mask;
	}
	for (index = 0; index < sizeof(analog) / sizeof(analog[0]); index++)
	{
		if (pad->buttons & analog[index].button)
			gamepad->bAnalogButtons[analog[index].index] = 0xff;
	}
	gamepad->sThumbLX = stick(pad->left_x);
	gamepad->sThumbLY = stick(pad->left_y);
	gamepad->sThumbRX = stick(pad->right_x);
	gamepad->sThumbRY = stick(pad->right_y);
}

DWORD WINAPI XInputGetState(HANDLE device, PXINPUT_STATE state)
{
	int port = controller_port(device);
	struct wiiu_pad pad;

	memset(state, 0, sizeof(*state));
	if (port < 0)
		return ERROR_DEVICE_NOT_CONNECTED;
	wiiu_pad_read(port, &pad);
	if (!pad.connected)
		return ERROR_DEVICE_NOT_CONNECTED;
	pad_state(&pad, &state->Gamepad);
	if (memcmp(&state->Gamepad, &controllers[port].previous, sizeof(state->Gamepad)))
	{
		controllers[port].packet_number++;
		controllers[port].previous = state->Gamepad;
	}
	state->dwPacketNumber = controllers[port].packet_number;
	return ERROR_SUCCESS;
}

DWORD WINAPI XInputSetState(HANDLE device, PXINPUT_FEEDBACK feedback)
{
	int port = controller_port(device);
	WORD strongest;

	if (!feedback)
		return ERROR_INVALID_PARAMETER;
	feedback->Header.dwStatus = ERROR_SUCCESS;
	if (port < 0)
		return ERROR_DEVICE_NOT_CONNECTED;
	strongest = feedback->Rumble.wLeftMotorSpeed > feedback->Rumble.wRightMotorSpeed ?
		feedback->Rumble.wLeftMotorSpeed : feedback->Rumble.wRightMotorSpeed;
	wiiu_pad_rumble(port, strongest / 65535.0f);
	return ERROR_SUCCESS;
}

DWORD WINAPI XInputDebugInitKeyboardQueue(PXINPUT_DEBUG_KEYQUEUE_PARAMETERS parameters)
{
	(void)parameters;
	return ERROR_SUCCESS;
}

DWORD WINAPI XInputDebugGetKeystroke(PXINPUT_DEBUG_KEYSTROKE keystroke)
{
	memset(keystroke, 0, sizeof(*keystroke));
	return ERROR_HANDLE_EOF;
}

/* ---------- the port's keyboard and mouse: none */

unsigned long halo_keyboard_actions(short controller_index)
{
	(void)controller_index;
	return 0;
}

int halo_linux_mouse_aiming(short gamepad_index)
{
	(void)gamepad_index;
	return 0;
}

int halo_linux_mouse_look(short gamepad_index, float *yaw, float *pitch)
{
	(void)gamepad_index;
	(void)yaw;
	(void)pitch;
	return 0;
}

void halo_input_name(int input, char *name, size_t size)
{
	(void)input;
	if (size)
		name[0] = 0;
}

void platform_text_field(int typing) { (void)typing; }
void platform_text_typing(int typing) { (void)typing; }
void test_input_hold_action(int hold) { (void)hold; }
