/*
WIIU_BINK_DECODE.H

A Bink movie's frames and sound, decoded by FFmpeg (wiiu_bink_decode.c).
*/

#ifndef __WIIU_BINK_DECODE_H
#define __WIIU_BINK_DECODE_H

struct wiiu_bink_movie;

struct wiiu_bink_information
{
	unsigned long width, height;
	unsigned long frames;          /* 0: not known */
	double frames_per_second;
	int channels;                  /* 0: no sound */
	int sample_rate;
};

/* bytes of 16-bit little-endian samples, interleaved */
typedef void (*wiiu_bink_sound_proc)(void *context, const void *samples, unsigned long size);

struct wiiu_bink_movie *wiiu_bink_open(const char *path, struct wiiu_bink_information *information);
void wiiu_bink_set_sound(struct wiiu_bink_movie *movie, wiiu_bink_sound_proc sound, void *context);
/* decodes up to the next video frame, handing the sound before it to the
sound callback; 0 once the movie has ended */
int wiiu_bink_decode_frame(struct wiiu_bink_movie *movie);
/* the newest frame, as X8R8G8B8 texels low byte first */
void wiiu_bink_copy_frame(struct wiiu_bink_movie *movie, unsigned char *destination, long pitch,
	unsigned long width, unsigned long height);
void wiiu_bink_close(struct wiiu_bink_movie *movie);

#endif
