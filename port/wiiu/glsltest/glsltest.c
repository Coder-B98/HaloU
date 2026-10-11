/*
GLSLTEST.C

A Wii U test of CafeGLSL, the shader compiler, without the game: the game
dies inside its first shader compile in Cemu (run r03b) and leaves no trace,
so this takes the same compile apart. Each step is written to
sd:/halou/glsltest.txt before it starts, so the last line says where it
died:

1. the compiler started, then the game's clear shaders compiled one at a
   time and as a pair, on the program's own thread, before any graphics;
2. the same pair after the graphics start (WHBGfxInit), as in the game;
3. the same pair on a thread like the game's (8 MB of stack, CPU 1).

The free memory is written before each step. The file is appended to, never
overwritten: each run starts with its own header line.
*/

#include <coreinit/memexpheap.h>
#include <coreinit/memheap.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <whb/gfx.h>
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_udp.h>
#include <whb/proc.h>

#include <cafeglsl/CafeGLSLCompiler.h>

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef HALOU_BUILD_ID
#include "halou_build_id.h"
#endif

#define LOG_PATH "fs:/vol/external01/halou/glsltest.txt"

static FILE *log_file;
static OSTime start_time;

/* the clear shaders of port/wiiu/src/wiiu_gl.c, as they are there */
static const char vertex_clear[] =
	"#version 310 es\n"
	"precision highp float;\n"
	"uniform vec4 rectangle;\n"
	"uniform vec4 depth;\n"
	"void main()\n"
	"{\n"
	"\tint corner = gl_VertexID;\n"
	"\tgl_Position = vec4((corner & 1) != 0 ? rectangle.z : rectangle.x,\n"
	"\t\t(corner & 2) != 0 ? rectangle.w : rectangle.y, depth.x, 1.0);\n"
	"}\n";

static const char pixel_clear[] =
	"#version 310 es\n"
	"precision highp float;\n"
	"uniform vec4 color;\n"
	"layout(location = 0) out vec4 result;\n"
	"void main()\n"
	"{\n"
	"\tresult = color;\n"
	"}\n";

/* a line of the log, with the seconds since start; flushed at once, so a
crash leaves it */
static void say(const char *format, ...) __attribute__((format(printf, 1, 2)));
static void say(const char *format, ...)
{
	char line[512];
	unsigned long milliseconds = (unsigned long)OSTicksToMilliseconds(OSGetSystemTime() - start_time);
	va_list arguments;

	va_start(arguments, format);
	vsnprintf(line, sizeof(line), format, arguments);
	va_end(arguments);
	WHBLogPrintf("glsltest: %s", line);
	if (log_file)
	{
		fprintf(log_file, "[%5lu.%03lu] %s\n", milliseconds / 1000, milliseconds % 1000, line);
		fflush(log_file);
	}
}

static void say_memory(const char *when)
{
	MEMHeapHandle heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM2);

	say("memory %s: %lu KB free, largest block %lu KB", when,
		(unsigned long)MEMGetTotalFreeSizeForExpHeap(heap) / 1024,
		(unsigned long)MEMGetAllocatableSizeForExpHeapEx(heap, 4) / 1024);
}

static int compile_pair(const char *where)
{
	GX2VertexShader *vertex = NULL;
	GX2PixelShader *pixel = NULL;
	char info[1024];

	info[0] = 0;
	say_memory(where);
	say("%s: compiling the clear pair", where);
	if (CompileShaderPair(vertex_clear, pixel_clear, GLSL_COMPILE_REGISTER, &vertex, &pixel, info, sizeof(info),
		GLSL_COMPILER_FLAG_NONE) == GLSL_SHADER_MODE_ERROR)
	{
		say("%s: FAILED: %s", where, info);
		return 0;
	}
	say("%s: OK (%u + %u bytes)", where, (unsigned)vertex->size, (unsigned)pixel->size);
	return 1;
}

static void *thread_test(void *unused)
{
	(void)unused;
	OSSetThreadAffinity(OSGetCurrentThread(), OS_THREAD_ATTRIB_AFFINITY_CPU1);
	OSYieldThread();
	compile_pair("step 3 (8 MB thread, CPU 1)");
	return NULL;
}

int main(int argc, char **argv)
{
	GX2VertexShader *vertex;
	GX2PixelShader *pixel;
	char info[1024];
	pthread_attr_t attributes;
	pthread_t thread;

	(void)argc;
	(void)argv;
	start_time = OSGetSystemTime();
	WHBProcInit();
	WHBLogCafeInit();
	WHBLogUdpInit();
	log_file = fopen(LOG_PATH, "a");
	say("=== glsltest, build " HALOU_BUILD_ID " ===");

	say_memory("at start");
	say("step 1: starting the compiler");
	InitGLSLCompiler();
	say("step 1: compiler %s started", GetGLSLCompilerVersion());

	info[0] = 0;
	say("step 1: compiling the clear vertex shader alone");
	vertex = NULL;
	CompileVertexShader(vertex_clear, GLSL_COMPILE_REGISTER, &vertex, info, sizeof(info), GLSL_COMPILER_FLAG_NONE);
	say("step 1: vertex shader %s %s", vertex ? "OK" : "FAILED", info);
	info[0] = 0;
	say("step 1: compiling the clear pixel shader alone");
	pixel = NULL;
	CompilePixelShader(pixel_clear, GLSL_COMPILE_REGISTER, &pixel, info, sizeof(info), GLSL_COMPILER_FLAG_NONE);
	say("step 1: pixel shader %s %s", pixel ? "OK" : "FAILED", info);
	compile_pair("step 1 (main thread, no graphics)");

	say("step 2: starting the graphics (WHBGfxInit)");
	WHBGfxInit();
	compile_pair("step 2 (main thread, graphics on)");

	pthread_attr_init(&attributes);
	pthread_attr_setstacksize(&attributes, 8 * 1024 * 1024);
	if (pthread_create(&thread, &attributes, thread_test, NULL) == 0)
		pthread_join(thread, NULL);
	else
		say("step 3: cannot start the thread");
	pthread_attr_destroy(&attributes);

	say("DONE: every step finished. Press HOME to quit.");
	if (log_file)
		fclose(log_file);
	log_file = NULL;
	while (WHBProcIsRunning())
	{
		WHBGfxBeginRender();
		WHBGfxBeginRenderTV();
		WHBGfxClearColor(0.0f, 0.4f, 0.0f, 1.0f);
		WHBGfxFinishRenderTV();
		WHBGfxBeginRenderDRC();
		WHBGfxClearColor(0.0f, 0.4f, 0.0f, 1.0f);
		WHBGfxFinishRenderDRC();
		WHBGfxFinishRender();
	}
	WHBGfxShutdown();
	WHBLogUdpDeinit();
	WHBLogCafeDeinit();
	WHBProcShutdown();
	return 0;
}
