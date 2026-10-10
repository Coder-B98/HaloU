/*
WIIU_DSOUND_NULL.C

DirectSound with nothing heard, for the Wii U build until it has a sound
mixer (port/linux/src/dsound_sdl.c is the Linux build's). The game's sound
streams take its packets and hand them back as played, one game frame
later (DirectSoundDoWork), so the sound code runs as it would with sound.
*/

#include "platform.h"

#include <string.h>

/* ---------- streams */

#define MAXIMUM_STREAM_PACKETS 64

struct null_stream
{
	/* must be first: in C an IDirectSoundStream is just { lpVtbl } */
	IDirectSoundStream object;
	struct null_stream *next;
	ULONG reference_count;
	LPFNXMEDIAOBJECTCALLBACK callback;
	LPVOID context;
	XMEDIAPACKET packets[MAXIMUM_STREAM_PACKETS];
	unsigned long packet_head;
	unsigned long packet_count;
};

static pthread_mutex_t streams_lock = PTHREAD_MUTEX_INITIALIZER;
static struct null_stream *streams;

static struct null_stream *stream_from_interface(void *stream)
{
	return (struct null_stream *)stream;
}

/* hands the oldest packet back; under streams_lock, which the callback
runs outside of */
static void stream_complete_head(struct null_stream *stream, DWORD status)
{
	XMEDIAPACKET packet = stream->packets[stream->packet_head];

	stream->packet_head = (stream->packet_head + 1) % MAXIMUM_STREAM_PACKETS;
	stream->packet_count--;
	if (packet.pdwCompletedSize)
		*packet.pdwCompletedSize = status == XMEDIAPACKET_STATUS_SUCCESS ? packet.dwMaxSize : 0;
	if (packet.pdwStatus)
		*packet.pdwStatus = status;
	if (stream->callback)
	{
		pthread_mutex_unlock(&streams_lock);
		stream->callback(stream->context, packet.pContext, status);
		pthread_mutex_lock(&streams_lock);
	}
	else if (packet.hCompletionEvent)
	{
		SetEvent(packet.hCompletionEvent);
	}
}

static ULONG STDMETHODCALLTYPE stream_add_reference(IDirectSoundStream *object)
{
	struct null_stream *stream = stream_from_interface(object);
	ULONG count;

	pthread_mutex_lock(&streams_lock);
	count = ++stream->reference_count;
	pthread_mutex_unlock(&streams_lock);
	return count;
}

static HRESULT STDMETHODCALLTYPE stream_flush(IDirectSoundStream *object)
{
	struct null_stream *stream = stream_from_interface(object);

	pthread_mutex_lock(&streams_lock);
	while (stream->packet_count)
		stream_complete_head(stream, XMEDIAPACKET_STATUS_FLUSHED);
	pthread_mutex_unlock(&streams_lock);
	return S_OK;
}

static ULONG STDMETHODCALLTYPE stream_release(IDirectSoundStream *object)
{
	struct null_stream *stream = stream_from_interface(object);
	struct null_stream **link;
	ULONG count;

	pthread_mutex_lock(&streams_lock);
	count = --stream->reference_count;
	pthread_mutex_unlock(&streams_lock);
	if (count)
		return count;
	stream_flush(object);
	pthread_mutex_lock(&streams_lock);
	for (link = &streams; *link; link = &(*link)->next)
	{
		if (*link == stream)
		{
			*link = stream->next;
			break;
		}
	}
	pthread_mutex_unlock(&streams_lock);
	free(stream);
	return 0;
}

static HRESULT STDMETHODCALLTYPE stream_get_info(IDirectSoundStream *object, LPXMEDIAINFO information)
{
	(void)object;
	memset(information, 0, sizeof(*information));
	information->dwFlags = XMO_STREAMF_FIXED_SAMPLE_SIZE | XMO_STREAMF_INPUT_ASYNC;
	information->dwInputSize = 4;
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE stream_get_status(IDirectSoundStream *object, LPDWORD status)
{
	struct null_stream *stream = stream_from_interface(object);

	pthread_mutex_lock(&streams_lock);
	*status = stream->packet_count < MAXIMUM_STREAM_PACKETS ? XMO_STATUSF_ACCEPT_INPUT_DATA : 0;
	pthread_mutex_unlock(&streams_lock);
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE stream_process(IDirectSoundStream *object, LPCXMEDIAPACKET input, LPCXMEDIAPACKET output)
{
	struct null_stream *stream = stream_from_interface(object);

	(void)output;
	if (!input)
		return E_INVALIDARG;
	pthread_mutex_lock(&streams_lock);
	if (stream->packet_count == MAXIMUM_STREAM_PACKETS)
	{
		pthread_mutex_unlock(&streams_lock);
		return E_OUTOFMEMORY;
	}
	stream->packets[(stream->packet_head + stream->packet_count) % MAXIMUM_STREAM_PACKETS] = *input;
	stream->packet_count++;
	if (input->pdwStatus)
		*input->pdwStatus = XMEDIAPACKET_STATUS_PENDING;
	if (input->pdwCompletedSize)
		*input->pdwCompletedSize = 0;
	pthread_mutex_unlock(&streams_lock);
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE stream_discontinuity(IDirectSoundStream *object)
{
	(void)object;
	return S_OK;
}

static IDirectSoundStreamVtbl stream_vtable =
{
	stream_add_reference,
	stream_release,
	stream_get_info,
	stream_get_status,
	stream_process,
	stream_discontinuity,
	stream_flush,
};

/* ---------- the DirectSound object */

static ULONG direct_sound_references;

HRESULT WINAPI DirectSoundCreate(LPGUID device_id, LPDIRECTSOUND *result, LPUNKNOWN outer)
{
	(void)device_id;
	(void)outer;
	if (!direct_sound_references++)
		platform_log("DirectSound: no sound yet on the Wii U (nothing is heard)");
	*result = (LPDIRECTSOUND)&direct_sound_references;
	return DS_OK;
}

ULONG WINAPI IDirectSound_Release(LPDIRECTSOUND sound)
{
	(void)sound;
	return direct_sound_references ? --direct_sound_references : 0;
}

/* each game frame: the packets given before it are played */
VOID WINAPI DirectSoundDoWork(void)
{
	struct null_stream *stream;

	pthread_mutex_lock(&streams_lock);
	for (stream = streams; stream; stream = stream->next)
	{
		/* (not those the callbacks give now: the next frame's) */
		unsigned long count = stream->packet_count;

		while (count-- && stream->packet_count)
			stream_complete_head(stream, XMEDIAPACKET_STATUS_SUCCESS);
	}
	pthread_mutex_unlock(&streams_lock);
}

VOID WINAPI DirectSoundUseFullHRTF(void)
{
}

HRESULT WINAPI IDirectSound_GetCaps(LPDIRECTSOUND sound, LPDSCAPS caps)
{
	(void)sound;
	memset(caps, 0, sizeof(*caps));
	caps->dwFree2DBuffers = 64;
	caps->dwFree3DBuffers = 64;
	caps->dwFreeBufferSGEs = 2047;
	return DS_OK;
}

HRESULT WINAPI IDirectSound_GetSpeakerConfig(LPDIRECTSOUND sound, LPDWORD speaker_config)
{
	(void)sound;
	*speaker_config = DSSPEAKER_STEREO;
	return DS_OK;
}

HRESULT WINAPI IDirectSound_DownloadEffectsImage(LPDIRECTSOUND sound, LPCVOID image, DWORD image_size,
	LPCDSEFFECTIMAGELOC image_location, LPDSEFFECTIMAGEDESC *image_description)
{
	(void)sound;
	(void)image;
	(void)image_size;
	(void)image_location;
	if (image_description)
		*image_description = NULL;
	return DS_OK;
}

HRESULT WINAPI IDirectSound_CreateSoundStream(LPDIRECTSOUND sound, LPCDSSTREAMDESC description,
	LPDIRECTSOUNDSTREAM *result, LPUNKNOWN outer)
{
	struct null_stream *stream = calloc(1, sizeof(*stream));

	(void)sound;
	(void)outer;
	if (!stream)
		return E_OUTOFMEMORY;
	stream->object.lpVtbl = &stream_vtable;
	stream->reference_count = 1;
	stream->callback = description->lpfnCallback;
	stream->context = description->lpvContext;
	pthread_mutex_lock(&streams_lock);
	stream->next = streams;
	streams = stream;
	pthread_mutex_unlock(&streams_lock);
	*result = &stream->object;
	return DS_OK;
}

HRESULT WINAPI IDirectSound_CommitDeferredSettings(LPDIRECTSOUND sound) { (void)sound; return DS_OK; }
HRESULT WINAPI IDirectSound_SetMixBinHeadroom(LPDIRECTSOUND sound, DWORD mix_bin_mask, DWORD headroom)
{
	(void)sound; (void)mix_bin_mask; (void)headroom;
	return DS_OK;
}
HRESULT WINAPI IDirectSound_SetI3DL2Listener(LPDIRECTSOUND sound, LPCDSI3DL2LISTENER listener_properties, DWORD apply)
{
	(void)sound; (void)listener_properties; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSound_SetDistanceFactor(LPDIRECTSOUND sound, FLOAT factor, DWORD apply)
{
	(void)sound; (void)factor; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSound_SetRolloffFactor(LPDIRECTSOUND sound, FLOAT factor, DWORD apply)
{
	(void)sound; (void)factor; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSound_SetPosition(LPDIRECTSOUND sound, FLOAT x, FLOAT y, FLOAT z, DWORD apply)
{
	(void)sound; (void)x; (void)y; (void)z; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSound_SetVelocity(LPDIRECTSOUND sound, FLOAT x, FLOAT y, FLOAT z, DWORD apply)
{
	(void)sound; (void)x; (void)y; (void)z; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSound_SetOrientation(LPDIRECTSOUND sound, FLOAT x_front, FLOAT y_front, FLOAT z_front,
	FLOAT x_top, FLOAT y_top, FLOAT z_top, DWORD apply)
{
	(void)sound; (void)x_front; (void)y_front; (void)z_front; (void)x_top; (void)y_top; (void)z_top; (void)apply;
	return DS_OK;
}

/* January-era DirectSound exports the game declares itself
(sound_dsound_xbox.c) */

void __stdcall DirectSoundStopStream(LPDIRECTSOUNDSTREAM stream)
{
	stream_flush(stream);
}

unsigned long __stdcall DirectSoundGetStreamVoiceStatus(LPDIRECTSOUNDSTREAM stream)
{
	unsigned long active;

	pthread_mutex_lock(&streams_lock);
	active = stream_from_interface(stream)->packet_count != 0;
	pthread_mutex_unlock(&streams_lock);
	return active;
}

/* ---------- stream settings: taken, and nothing done with them */

HRESULT WINAPI IDirectSoundStream_SetFrequency(LPDIRECTSOUNDSTREAM stream, DWORD frequency)
{
	(void)stream; (void)frequency;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetVolume(LPDIRECTSOUNDSTREAM stream, LONG volume)
{
	(void)stream; (void)volume;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetMixBins(LPDIRECTSOUNDSTREAM stream, DWORD mix_bin_mask)
{
	(void)stream; (void)mix_bin_mask;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetMixBinVolumes(LPDIRECTSOUNDSTREAM stream, DWORD mix_bin_mask, const LONG *volumes)
{
	(void)stream; (void)mix_bin_mask; (void)volumes;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetMode(LPDIRECTSOUNDSTREAM stream, DWORD mode, DWORD apply)
{
	(void)stream; (void)mode; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetPosition(LPDIRECTSOUNDSTREAM stream, FLOAT x, FLOAT y, FLOAT z, DWORD apply)
{
	(void)stream; (void)x; (void)y; (void)z; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetVelocity(LPDIRECTSOUNDSTREAM stream, FLOAT x, FLOAT y, FLOAT z, DWORD apply)
{
	(void)stream; (void)x; (void)y; (void)z; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetMinDistance(LPDIRECTSOUNDSTREAM stream, FLOAT distance, DWORD apply)
{
	(void)stream; (void)distance; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetMaxDistance(LPDIRECTSOUNDSTREAM stream, FLOAT distance, DWORD apply)
{
	(void)stream; (void)distance; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetI3DL2Source(LPDIRECTSOUNDSTREAM stream, LPCDSI3DL2BUFFER source, DWORD apply)
{
	(void)stream; (void)source; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetConeAngles(LPDIRECTSOUNDSTREAM stream, DWORD inside, DWORD outside, DWORD apply)
{
	(void)stream; (void)inside; (void)outside; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetConeOrientation(LPDIRECTSOUNDSTREAM stream, FLOAT x, FLOAT y, FLOAT z, DWORD apply)
{
	(void)stream; (void)x; (void)y; (void)z; (void)apply;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundStream_SetConeOutsideVolume(LPDIRECTSOUNDSTREAM stream, LONG volume, DWORD apply)
{
	(void)stream; (void)volume; (void)apply;
	return DS_OK;
}

/* ---------- buffers (the game makes them, but its sounds go to streams) */

struct null_buffer
{
	ULONG reference_count;
};

HRESULT WINAPI DirectSoundCreateBuffer(LPCDSBUFFERDESC description, LPDIRECTSOUNDBUFFER *result)
{
	struct null_buffer *buffer = calloc(1, sizeof(*buffer));

	(void)description;
	if (!buffer)
		return E_OUTOFMEMORY;
	buffer->reference_count = 1;
	*result = (LPDIRECTSOUNDBUFFER)buffer;
	return DS_OK;
}

HRESULT WINAPI IDirectSound_CreateSoundBuffer(LPDIRECTSOUND sound, LPCDSBUFFERDESC description,
	LPDIRECTSOUNDBUFFER *result, LPUNKNOWN outer)
{
	(void)sound;
	(void)outer;
	return DirectSoundCreateBuffer(description, result);
}

ULONG WINAPI IDirectSoundBuffer_Release(LPDIRECTSOUNDBUFFER buffer)
{
	struct null_buffer *record = (struct null_buffer *)buffer;
	ULONG count = --record->reference_count;

	if (!count)
		free(record);
	return count;
}

HRESULT WINAPI IDirectSoundBuffer_SetBufferData(LPDIRECTSOUNDBUFFER buffer, LPVOID data, DWORD size)
{
	(void)buffer; (void)data; (void)size;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundBuffer_Play(LPDIRECTSOUNDBUFFER buffer, DWORD reserved1, DWORD reserved2, DWORD flags)
{
	(void)buffer; (void)reserved1; (void)reserved2; (void)flags;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundBuffer_Stop(LPDIRECTSOUNDBUFFER buffer) { (void)buffer; return DS_OK; }
HRESULT WINAPI IDirectSoundBuffer_SetCurrentPosition(LPDIRECTSOUNDBUFFER buffer, DWORD play_cursor)
{
	(void)buffer; (void)play_cursor;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundBuffer_SetLoopRegion(LPDIRECTSOUNDBUFFER buffer, DWORD loop_start, DWORD loop_length)
{
	(void)buffer; (void)loop_start; (void)loop_length;
	return DS_OK;
}
HRESULT WINAPI IDirectSoundBuffer_SetPitch(LPDIRECTSOUNDBUFFER buffer, LONG pitch) { (void)buffer; (void)pitch; return DS_OK; }
HRESULT WINAPI IDirectSoundBuffer_SetVolume(LPDIRECTSOUNDBUFFER buffer, LONG volume) { (void)buffer; (void)volume; return DS_OK; }
