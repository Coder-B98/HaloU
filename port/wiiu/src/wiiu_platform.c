/*
WIIU_PLATFORM.C

The rest of what the game asks of the platform layer, for the Wii U build:
the desktop ports' window, clipboard and settings screens (sdl_platform.c),
their high resolution HUD, text and menus (hud_hires.c, text_hires.c,
menu_files.c), none of which the Wii U has yet, and the few C library calls
devkitPPC's newlib declares but does not have.
*/

#include "platform.h"
#include "halo_menus.h"
#include "text_hires.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

/* ---------- the window, the clipboard, the settings screens */

void platform_show_message(const char *title, const char *message)
{
	platform_log("%s: %s", title, message);
}

BOOL platform_offer_game_data(const char *destination)
{
	(void)destination;
	return FALSE;
}

void platform_request_quit(void)
{
	platform_log("quit asked for (the Wii U build quits with the HOME button)");
}

int halo_interpolation_enabled(void)
{
	return 0;
}

int platform_display_resolutions(long *widths, long *heights, int maximum)
{
	(void)widths;
	(void)heights;
	(void)maximum;
	return 0;
}

int platform_window_sizes(long *widths, long *heights, int maximum)
{
	(void)widths;
	(void)heights;
	(void)maximum;
	return 0;
}

void platform_display_apply(void)
{
}

int platform_clipboard_get(char *text, int size)
{
	if (size > 0)
		text[0] = 0;
	return 0;
}

void platform_clipboard_set(const char *text)
{
	(void)text;
}

void platform_binding_capture_begin(void)
{
}

int platform_binding_capture_poll(int *input)
{
	(void)input;
	return 0;
}

void platform_scoreboard_scroll(int open, long *notches, long *pages)
{
	(void)open;
	if (notches)
		*notches = 0;
	if (pages)
		*pages = 0;
}

/* ---------- high resolution HUD and text: none, so the maps' own bitmaps
and fonts are drawn */

long hud_hires_asset_count(void)
{
	return 0;
}

char const *hud_hires_asset_tag(long asset)
{
	(void)asset;
	return "";
}

long hud_hires_asset_bitmap(long asset)
{
	(void)asset;
	return -1;
}

long hud_hires_asset_fits(long asset, long width, long height)
{
	(void)asset;
	(void)width;
	(void)height;
	return 0;
}

long text_hires_font(char const *tag_name, float cap_height, float oversample)
{
	(void)tag_name;
	(void)cap_height;
	(void)oversample;
	return -1;
}

int text_hires_covers(long font, unsigned long code)
{
	(void)font;
	(void)code;
	return 0;
}

int text_hires_glyph(long font, unsigned long code, struct text_hires_glyph *glyph)
{
	(void)font;
	(void)code;
	(void)glyph;
	return 0;
}

void text_hires_register_atlas(const unsigned long *texture, unsigned long width, unsigned long height)
{
	(void)texture;
	(void)width;
	(void)height;
}

/* ---------- the PC menus' files: none, so the Xbox menus are used */

struct halo_menus const *halo_menus_load(void)
{
	return NULL;
}

/* (menu_files.c's) */
long halo_menus_utf16(char const *utf8, unsigned short *out, long capacity)
{
	const unsigned char *in = (const unsigned char *)utf8;
	long count = 0;

	while (*in && count < capacity - 1)
	{
		unsigned long code = *in++;

		if (code == '\\' && *in == 'n')
		{
			in++;
			if (count + 2 >= capacity)
				break;
			out[count++] = '\r';
			out[count++] = '\n';
			continue;
		}
		if (code >= 0xC0)
		{
			int more = code >= 0xF0 ? 3 : code >= 0xE0 ? 2 : 1;

			code &= 0x3F >> more;
			while (more-- && (*in & 0xC0) == 0x80)
				code = (code << 6) | (*in++ & 0x3F);
		}
		out[count++] = code < 0x10000 ? (unsigned short)code : '?';
	}
	out[count++] = 0;
	return count;
}

void halo_menus_log(char const *file, long line, char const *message, char const *detail)
{
	platform_log("menus: %s:%ld: %s%s%s", file ? file : "?", line, message, detail ? " " : "", detail ? detail : "");
}

void halo_menus_art_register(void const *texture, char const *png)
{
	(void)texture;
	(void)png;
}

void halo_menus_art_forget(void)
{
}

/* ---------- what newlib lacks */

/* MSVC's compiler barrier (scenario.c): no code, but the compiler may not
move memory accesses across it */
void _ReadWriteBarrier(void)
{
	__asm__ volatile("" ::: "memory");
}

/* (SleepEx's INFINITE: a thread that never wakes) */
int pause(void)
{
	for (;;)
		sleep(3600);
	return -1;
}

/* read and write at an offset, the file's own position kept: the Wii U's
file system has no such call, so the position is moved and moved back,
under a lock for the threads sharing a descriptor */
static pthread_mutex_t positioned_lock = PTHREAD_MUTEX_INITIALIZER;

static ssize_t positioned_io(int descriptor, void *read_buffer, const void *write_buffer, size_t count, off_t offset)
{
	off_t previous;
	ssize_t result = -1;

	pthread_mutex_lock(&positioned_lock);
	previous = lseek(descriptor, 0, SEEK_CUR);
	if (previous >= 0 && lseek(descriptor, offset, SEEK_SET) == offset)
	{
		int saved_errno;

		result = read_buffer ? read(descriptor, read_buffer, count) : write(descriptor, write_buffer, count);
		saved_errno = errno;
		lseek(descriptor, previous, SEEK_SET);
		errno = saved_errno;
	}
	pthread_mutex_unlock(&positioned_lock);
	return result;
}

ssize_t pread(int descriptor, void *buffer, size_t count, off_t offset)
{
	return positioned_io(descriptor, buffer, NULL, count, offset);
}

ssize_t pwrite(int descriptor, const void *buffer, size_t count, off_t offset)
{
	return positioned_io(descriptor, NULL, buffer, count, offset);
}
