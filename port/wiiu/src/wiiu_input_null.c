/*
WIIU_INPUT_NULL.C

Xbox controllers and keyboard for the Wii U build until it reads the
GamePad (port/linux/src/xinput_sdl.c is the Linux build's): none are
plugged in, so the game sits at its first screen waiting for one.
*/

#include "platform.h"
#include "halo_keyboard.h"

#include <string.h>

XPP_DEVICE_TYPE XDEVICE_TYPE_GAMEPAD_TABLE;
XPP_DEVICE_TYPE XDEVICE_TYPE_MEMORY_UNIT_TABLE;
XPP_DEVICE_TYPE XDEVICE_TYPE_DEBUG_KEYBOARD_TABLE;

VOID WINAPI XInitDevices(DWORD preallocation_type_count, PXDEVICE_PREALLOC_TYPE preallocation_types)
{
	(void)preallocation_type_count;
	(void)preallocation_types;
}

BOOL WINAPI XGetDeviceChanges(PXPP_DEVICE_TYPE device_type, PDWORD insertions, PDWORD removals)
{
	(void)device_type;
	*insertions = 0;
	*removals = 0;
	return FALSE;
}

HANDLE WINAPI XInputOpen(PXPP_DEVICE_TYPE device_type, DWORD port, DWORD slot,
	PXINPUT_POLLING_PARAMETERS polling_parameters)
{
	(void)device_type;
	(void)port;
	(void)slot;
	(void)polling_parameters;
	SetLastError(ERROR_DEVICE_NOT_CONNECTED);
	return NULL;
}

VOID WINAPI XInputClose(HANDLE device)
{
	(void)device;
}

DWORD WINAPI XInputGetState(HANDLE device, PXINPUT_STATE state)
{
	(void)device;
	memset(state, 0, sizeof(*state));
	return ERROR_DEVICE_NOT_CONNECTED;
}

DWORD WINAPI XInputSetState(HANDLE device, PXINPUT_FEEDBACK feedback)
{
	(void)device;
	(void)feedback;
	return ERROR_DEVICE_NOT_CONNECTED;
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
