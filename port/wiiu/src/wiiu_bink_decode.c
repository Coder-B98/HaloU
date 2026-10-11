/*
WIIU_BINK_DECODE.C

The Wii U build's Bink movie decoding, with FFmpeg's Bink demuxer and
decoders (tools/wiiu_ffmpeg.sh builds them). Kept apart from the game's
headers, whose Xbox names FFmpeg's would collide with: wiiu_bink.c, the
Bink SDK the game calls, uses this through wiiu_bink_decode.h.

A movie gives its frames one at a time, as FFmpeg's YUV 4:2:0, turned into
the X8R8G8B8 texels the game's movie texture is (low byte first: blue,
green, red, then an unused byte, xbox_textures.c), and its sound as 16-bit
interleaved samples, little-endian (dsound_sdl.c's PCM).
*/

#include "wiiu_bink_decode.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>

#include <stdlib.h>
#include <string.h>

struct wiiu_bink_movie
{
	AVFormatContext *format;
	AVCodecContext *video;
	AVCodecContext *audio;
	int video_stream, audio_stream;
	AVPacket *packet;
	AVFrame *frame;
	AVFrame *picture;   /* the newest video frame decoded */
	int have_picture;
	int ended;
	wiiu_bink_sound_proc sound;
	void *sound_context;
	short *samples;     /* sound converted for the callback */
	int sample_capacity;
};

static AVCodecContext *open_decoder(AVStream *stream)
{
	const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
	AVCodecContext *context;

	if (!codec)
		return NULL;
	context = avcodec_alloc_context3(codec);
	if (!context)
		return NULL;
	if (avcodec_parameters_to_context(context, stream->codecpar) < 0 || avcodec_open2(context, codec, NULL) < 0)
	{
		avcodec_free_context(&context);
		return NULL;
	}
	return context;
}

struct wiiu_bink_movie *wiiu_bink_open(const char *path, struct wiiu_bink_information *information)
{
	struct wiiu_bink_movie *movie = calloc(1, sizeof(*movie));
	char url[1100];
	AVStream *stream;
	unsigned int index;

	if (!movie)
		return NULL;
	movie->video_stream = movie->audio_stream = -1;
	/* (the file protocol by name: "fs:/vol/..." would read as a protocol) */
	snprintf(url, sizeof(url), "file:%s", path);
	if (avformat_open_input(&movie->format, url, NULL, NULL) < 0)
	{
		free(movie);
		return NULL;
	}
	if (avformat_find_stream_info(movie->format, NULL) < 0)
		goto failed;
	for (index = 0; index < movie->format->nb_streams; index++)
	{
		enum AVMediaType type = movie->format->streams[index]->codecpar->codec_type;

		if (type == AVMEDIA_TYPE_VIDEO && movie->video_stream < 0)
			movie->video_stream = (int)index;
		else if (type == AVMEDIA_TYPE_AUDIO && movie->audio_stream < 0)
			movie->audio_stream = (int)index;
	}
	if (movie->video_stream < 0)
		goto failed;
	stream = movie->format->streams[movie->video_stream];
	movie->video = open_decoder(stream);
	if (!movie->video)
		goto failed;
	if (movie->audio_stream >= 0)
	{
		movie->audio = open_decoder(movie->format->streams[movie->audio_stream]);
		if (!movie->audio)
			movie->audio_stream = -1;
	}
	movie->packet = av_packet_alloc();
	movie->frame = av_frame_alloc();
	movie->picture = av_frame_alloc();
	if (!movie->packet || !movie->frame || !movie->picture)
		goto failed;

	memset(information, 0, sizeof(*information));
	information->width = movie->video->width;
	information->height = movie->video->height;
	/* (the Bink demuxer gives the count as the stream's duration, in frames) */
	information->frames = stream->nb_frames > 0 ? (unsigned long)stream->nb_frames :
		stream->duration > 0 ? (unsigned long)stream->duration : 0;
	information->frames_per_second = stream->avg_frame_rate.den ? av_q2d(stream->avg_frame_rate) :
		stream->r_frame_rate.den ? av_q2d(stream->r_frame_rate) : 30.0;
	if (movie->audio)
	{
		information->channels = movie->audio->ch_layout.nb_channels;
		information->sample_rate = movie->audio->sample_rate;
	}
	return movie;

failed:
	wiiu_bink_close(movie);
	return NULL;
}

void wiiu_bink_set_sound(struct wiiu_bink_movie *movie, wiiu_bink_sound_proc sound, void *context)
{
	movie->sound = sound;
	movie->sound_context = context;
}

static float sample_at(const AVFrame *frame, int channel, int index)
{
	if (frame->format == AV_SAMPLE_FMT_FLTP)
		return ((const float *)frame->extended_data[channel])[index];
	if (frame->format == AV_SAMPLE_FMT_FLT)
		return ((const float *)frame->data[0])[index * frame->ch_layout.nb_channels + channel];
	if (frame->format == AV_SAMPLE_FMT_S16P)
		return ((const short *)frame->extended_data[channel])[index] / 32768.0f;
	if (frame->format == AV_SAMPLE_FMT_S16)
		return ((const short *)frame->data[0])[index * frame->ch_layout.nb_channels + channel] / 32768.0f;
	return 0.0f;
}

/* a decoded block of sound, to the callback as little-endian 16-bit */
static void deliver_sound(struct wiiu_bink_movie *movie, const AVFrame *frame)
{
	int channels = frame->ch_layout.nb_channels;
	int count = frame->nb_samples * channels;
	int index;

	if (!movie->sound || channels <= 0 || frame->nb_samples <= 0)
		return;
	if (count > movie->sample_capacity)
	{
		short *samples = realloc(movie->samples, (size_t)count * sizeof(short));

		if (!samples)
			return;
		movie->samples = samples;
		movie->sample_capacity = count;
	}
	for (index = 0; index < count; index++)
	{
		float value = sample_at(frame, index % channels, index / channels) * 32767.0f;
		int integer = value > 32767.0f ? 32767 : value < -32768.0f ? -32768 : (int)value;
		unsigned char *bytes = (unsigned char *)&movie->samples[index];

		bytes[0] = (unsigned char)(integer & 0xff);
		bytes[1] = (unsigned char)((integer >> 8) & 0xff);
	}
	movie->sound(movie->sound_context, movie->samples, (unsigned long)count * sizeof(short));
}

int wiiu_bink_decode_frame(struct wiiu_bink_movie *movie)
{
	while (!movie->ended)
	{
		int result = av_read_frame(movie->format, movie->packet);

		if (result < 0)
		{
			movie->ended = 1;
			break;
		}
		if (movie->packet->stream_index == movie->audio_stream && movie->audio)
		{
			if (avcodec_send_packet(movie->audio, movie->packet) >= 0)
			{
				while (avcodec_receive_frame(movie->audio, movie->frame) >= 0)
				{
					deliver_sound(movie, movie->frame);
					av_frame_unref(movie->frame);
				}
			}
			av_packet_unref(movie->packet);
		}
		else if (movie->packet->stream_index == movie->video_stream)
		{
			int got = 0;

			if (avcodec_send_packet(movie->video, movie->packet) >= 0 &&
				avcodec_receive_frame(movie->video, movie->frame) >= 0)
			{
				av_frame_unref(movie->picture);
				av_frame_move_ref(movie->picture, movie->frame);
				movie->have_picture = 1;
				got = 1;
			}
			av_packet_unref(movie->packet);
			if (got)
				return 1;
		}
		else
		{
			av_packet_unref(movie->packet);
		}
	}
	return 0;
}

static unsigned char clamp_byte(int value)
{
	return (unsigned char)(value < 0 ? 0 : value > 255 ? 255 : value);
}

void wiiu_bink_copy_frame(struct wiiu_bink_movie *movie, unsigned char *destination, long pitch,
	unsigned long width, unsigned long height)
{
	const AVFrame *picture = movie->picture;
	int full_range = movie->video->color_range == AVCOL_RANGE_JPEG;
	unsigned long x, y;

	if (!movie->have_picture)
		return;
	if (width > (unsigned long)picture->width)
		width = (unsigned long)picture->width;
	if (height > (unsigned long)picture->height)
		height = (unsigned long)picture->height;
	for (y = 0; y < height; y++)
	{
		const unsigned char *luma = picture->data[0] + y * picture->linesize[0];
		const unsigned char *blue_difference = picture->data[1] + (y / 2) * picture->linesize[1];
		const unsigned char *red_difference = picture->data[2] + (y / 2) * picture->linesize[2];
		unsigned char *texel = destination + y * pitch;

		for (x = 0; x < width; x++)
		{
			int c = full_range ? luma[x] : (luma[x] - 16) * 298 / 256;
			int d = blue_difference[x / 2] - 128;
			int e = red_difference[x / 2] - 128;

			if (full_range)
			{
				texel[0] = clamp_byte(c + ((454 * d + 128) >> 8));
				texel[1] = clamp_byte(c - ((88 * d + 183 * e + 128) >> 8));
				texel[2] = clamp_byte(c + ((359 * e + 128) >> 8));
			}
			else
			{
				texel[0] = clamp_byte(c + ((516 * d + 128) >> 8));
				texel[1] = clamp_byte(c - ((100 * d + 208 * e + 128) >> 8));
				texel[2] = clamp_byte(c + ((409 * e + 128) >> 8));
			}
			texel[3] = 0xff;
			texel += 4;
		}
	}
}

void wiiu_bink_close(struct wiiu_bink_movie *movie)
{
	if (!movie)
		return;
	av_packet_free(&movie->packet);
	av_frame_free(&movie->frame);
	av_frame_free(&movie->picture);
	avcodec_free_context(&movie->video);
	avcodec_free_context(&movie->audio);
	avformat_close_input(&movie->format);
	free(movie->samples);
	free(movie);
}
