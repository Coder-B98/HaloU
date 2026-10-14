/*
WIIU_PADS.C

The Wii U's controllers, read for wiiu_input.c (which gives them to the game
as Xbox controllers): the GamePad through VPAD, and Pro and Classic
Controllers through KPAD. A read with no new sample keeps the last state, so
a button held stays held between the controller's own reports.
*/

#include <padscore/kpad.h>
#include <padscore/wpad.h>
#include <vpad/input.h>

#include <string.h>

#include "wiiu_pads.h"

static struct wiiu_pad last[WIIU_PAD_PORTS];
static int rumbling[WIIU_PAD_PORTS];

void wiiu_pads_initialize(void)
{
	static int started;

	if (started)
		return;
	started = 1;
	VPADInit();
	KPADInit();
	/* (the Pro Controller reports only once this is on) */
	WPADEnableURCC(1);
}

static const struct
{
	unsigned int from, to;
} gamepad_buttons[] =
{
	{ VPAD_BUTTON_B, WIIU_PAD_SOUTH }, { VPAD_BUTTON_A, WIIU_PAD_EAST },
	{ VPAD_BUTTON_Y, WIIU_PAD_WEST }, { VPAD_BUTTON_X, WIIU_PAD_NORTH },
	{ VPAD_BUTTON_UP, WIIU_PAD_UP }, { VPAD_BUTTON_DOWN, WIIU_PAD_DOWN },
	{ VPAD_BUTTON_LEFT, WIIU_PAD_LEFT }, { VPAD_BUTTON_RIGHT, WIIU_PAD_RIGHT },
	{ VPAD_BUTTON_PLUS, WIIU_PAD_PLUS }, { VPAD_BUTTON_MINUS, WIIU_PAD_MINUS },
	{ VPAD_BUTTON_L, WIIU_PAD_L }, { VPAD_BUTTON_R, WIIU_PAD_R },
	{ VPAD_BUTTON_ZL, WIIU_PAD_ZL }, { VPAD_BUTTON_ZR, WIIU_PAD_ZR },
	{ VPAD_BUTTON_STICK_L, WIIU_PAD_STICK_L }, { VPAD_BUTTON_STICK_R, WIIU_PAD_STICK_R },
};

static const struct
{
	unsigned int from, to;
} pro_buttons[] =
{
	{ WPAD_PRO_BUTTON_B, WIIU_PAD_SOUTH }, { WPAD_PRO_BUTTON_A, WIIU_PAD_EAST },
	{ WPAD_PRO_BUTTON_Y, WIIU_PAD_WEST }, { WPAD_PRO_BUTTON_X, WIIU_PAD_NORTH },
	{ WPAD_PRO_BUTTON_UP, WIIU_PAD_UP }, { WPAD_PRO_BUTTON_DOWN, WIIU_PAD_DOWN },
	{ WPAD_PRO_BUTTON_LEFT, WIIU_PAD_LEFT }, { WPAD_PRO_BUTTON_RIGHT, WIIU_PAD_RIGHT },
	{ WPAD_PRO_BUTTON_PLUS, WIIU_PAD_PLUS }, { WPAD_PRO_BUTTON_MINUS, WIIU_PAD_MINUS },
	{ WPAD_PRO_BUTTON_L, WIIU_PAD_L }, { WPAD_PRO_BUTTON_R, WIIU_PAD_R },
	{ WPAD_PRO_BUTTON_ZL, WIIU_PAD_ZL }, { WPAD_PRO_BUTTON_ZR, WIIU_PAD_ZR },
	{ WPAD_PRO_BUTTON_STICK_L, WIIU_PAD_STICK_L }, { WPAD_PRO_BUTTON_STICK_R, WIIU_PAD_STICK_R },
};

static const struct
{
	unsigned int from, to;
} classic_buttons[] =
{
	{ WPAD_CLASSIC_BUTTON_B, WIIU_PAD_SOUTH }, { WPAD_CLASSIC_BUTTON_A, WIIU_PAD_EAST },
	{ WPAD_CLASSIC_BUTTON_Y, WIIU_PAD_WEST }, { WPAD_CLASSIC_BUTTON_X, WIIU_PAD_NORTH },
	{ WPAD_CLASSIC_BUTTON_UP, WIIU_PAD_UP }, { WPAD_CLASSIC_BUTTON_DOWN, WIIU_PAD_DOWN },
	{ WPAD_CLASSIC_BUTTON_LEFT, WIIU_PAD_LEFT }, { WPAD_CLASSIC_BUTTON_RIGHT, WIIU_PAD_RIGHT },
	{ WPAD_CLASSIC_BUTTON_PLUS, WIIU_PAD_PLUS }, { WPAD_CLASSIC_BUTTON_MINUS, WIIU_PAD_MINUS },
	{ WPAD_CLASSIC_BUTTON_L, WIIU_PAD_L }, { WPAD_CLASSIC_BUTTON_R, WIIU_PAD_R },
	{ WPAD_CLASSIC_BUTTON_ZL, WIIU_PAD_ZL }, { WPAD_CLASSIC_BUTTON_ZR, WIIU_PAD_ZR },
};

#define MAP(table, held, result) \
	do { \
		unsigned int index_; \
		for (index_ = 0; index_ < sizeof(table) / sizeof(table[0]); index_++) \
			if ((held) & table[index_].from) \
				(result) |= table[index_].to; \
	} while (0)

static void gamepad_read(struct wiiu_pad *pad)
{
	VPADStatus status;
	VPADReadError error = VPAD_READ_SUCCESS;

	memset(&status, 0, sizeof(status));
	if (VPADRead(VPAD_CHAN_0, &status, 1, &error) < 1)
	{
		/* no new sample: the last one stands, unless the GamePad is gone */
		if (error == VPAD_READ_INVALID_CONTROLLER)
			memset(pad, 0, sizeof(*pad));
		return;
	}
	memset(pad, 0, sizeof(*pad));
	pad->connected = 1;
	MAP(gamepad_buttons, status.hold, pad->buttons);
	pad->left_x = status.leftStick.x;
	pad->left_y = status.leftStick.y;
	pad->right_x = status.rightStick.x;
	pad->right_y = status.rightStick.y;
}

static void remote_read(int channel, struct wiiu_pad *pad)
{
	KPADStatus status;
	KPADError error = KPAD_ERROR_OK;

	memset(&status, 0, sizeof(status));
	if (KPADReadEx((KPADChan)channel, &status, 1, &error) < 1)
	{
		if (error == KPAD_ERROR_INVALID_CONTROLLER)
			memset(pad, 0, sizeof(*pad));
		return;
	}
	memset(pad, 0, sizeof(*pad));
	if (status.extensionType == WPAD_EXT_PRO_CONTROLLER)
	{
		pad->connected = 1;
		MAP(pro_buttons, status.pro.hold, pad->buttons);
		pad->left_x = status.pro.leftStick.x;
		pad->left_y = status.pro.leftStick.y;
		pad->right_x = status.pro.rightStick.x;
		pad->right_y = status.pro.rightStick.y;
	}
	else if (status.extensionType == WPAD_EXT_CLASSIC)
	{
		pad->connected = 1;
		MAP(classic_buttons, status.classic.hold, pad->buttons);
		pad->left_x = status.classic.leftStick.x;
		pad->left_y = status.classic.leftStick.y;
		pad->right_x = status.classic.rightStick.x;
		pad->right_y = status.classic.rightStick.y;
	}
	/* (a Wii remote alone, or with a nunchuk, has too few buttons for Halo) */
}

void wiiu_pad_read(int port, struct wiiu_pad *pad)
{
	if (port < 0 || port >= WIIU_PAD_PORTS)
	{
		memset(pad, 0, sizeof(*pad));
		return;
	}
	wiiu_pads_initialize();
	if (port == 0)
		gamepad_read(&last[0]);
	else
		remote_read(port - 1, &last[port]);
	*pad = last[port];
}

void wiiu_pad_rumble(int port, float strength)
{
	/* the GamePad's motor plays a pattern, a byte at a time (0xff on), and
	stops at its end: the game asks again every frame while it rumbles */
	static uint8_t pattern[15] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	int on = strength > 0.2f;

	if (port < 0 || port >= WIIU_PAD_PORTS || (!on && !rumbling[port]))
		return;
	if (port == 0)
	{
		if (on)
			VPADControlMotor(VPAD_CHAN_0, pattern, sizeof(pattern));
		else
			VPADStopMotor(VPAD_CHAN_0);
	}
	else if (on != rumbling[port])
		WPADControlMotor((WPADChan)(port - 1), on);
	rumbling[port] = on;
}
