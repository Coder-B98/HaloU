/*
WIIU_PADS.H

The Wii U's controllers as the game's input layer (wiiu_input.c) sees them,
read by wiiu_pads.c. Neither wut's headers nor the Xbox's are here, since the
two cannot meet in one file (both declare BOOL).
*/

#ifndef WIIU_PADS_H
#define WIIU_PADS_H

/* port 0 is the GamePad; ports 1 to 3 are the first three Wii remote
channels, when a Pro Controller or a Classic Controller is on them */
#define WIIU_PAD_PORTS 4

/* the buttons, by where they sit, not by their letters: the bottom face
button is SOUTH (the Wii U's B, where the Xbox has A) */
enum
{
	WIIU_PAD_SOUTH = 1 << 0,
	WIIU_PAD_EAST = 1 << 1,
	WIIU_PAD_WEST = 1 << 2,
	WIIU_PAD_NORTH = 1 << 3,
	WIIU_PAD_UP = 1 << 4,
	WIIU_PAD_DOWN = 1 << 5,
	WIIU_PAD_LEFT = 1 << 6,
	WIIU_PAD_RIGHT = 1 << 7,
	WIIU_PAD_PLUS = 1 << 8,
	WIIU_PAD_MINUS = 1 << 9,
	WIIU_PAD_L = 1 << 10,
	WIIU_PAD_R = 1 << 11,
	WIIU_PAD_ZL = 1 << 12,
	WIIU_PAD_ZR = 1 << 13,
	WIIU_PAD_STICK_L = 1 << 14,
	WIIU_PAD_STICK_R = 1 << 15,
};

struct wiiu_pad
{
	int connected;
	unsigned int buttons;
	/* -1 to 1, up and right positive */
	float left_x, left_y, right_x, right_y;
};

void wiiu_pads_initialize(void);
/* the latest state of a port's controller */
void wiiu_pad_read(int port, struct wiiu_pad *pad);
/* the motors on (the strength of the stronger one, 0 to 1) or off */
void wiiu_pad_rumble(int port, float strength);

#endif
