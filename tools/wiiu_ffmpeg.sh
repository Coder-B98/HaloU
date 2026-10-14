#!/bin/bash
# builds the part of FFmpeg (ffmpeg.org) the Wii U build plays the game's
# Bink movies with (port/wiiu/src/wiiu_bink.c): the Bink demuxer and its
# video and audio decoders, nothing else, as static libraries
# (libavformat, libavcodec, libavutil), at the release BUILDING-WIIU.md
# pins, and installs them into devkitPro's Wii U portlibs. FFmpeg is LGPL
# 2.1 or later; this configuration enables no GPL parts. Needs devkitPro in
# $DEVKITPRO. Usage:
#     tools/wiiu_ffmpeg.sh [work folder]
set -e
TAG=n7.1
COMMIT=b08d7969c550a804a59511c7b83f2dd8cc0499b8
DEVKITPRO=${DEVKITPRO:-/opt/devkitpro}
WORK=${1:-build/ffmpeg}
mkdir -p "$WORK"
WORK=$(cd "$WORK" && pwd)

if [ ! -d "$WORK/src/.git" ]; then
	git clone -q --depth 1 --branch $TAG https://github.com/FFmpeg/FFmpeg.git "$WORK/src"
fi
test "$(git -C "$WORK/src" rev-parse HEAD)" = $COMMIT

export PATH="$DEVKITPRO/devkitPPC/bin:$PATH"
rm -rf "$WORK/build"
mkdir -p "$WORK/build"
cd "$WORK/build"
"$WORK/src/configure" --prefix="$DEVKITPRO/portlibs/wiiu" \
	--enable-cross-compile --arch=ppc --cpu=750 --target-os=none \
	--cross-prefix=powerpc-eabi- --cc=powerpc-eabi-gcc \
	--disable-everything --disable-programs --disable-doc \
	--disable-avdevice --disable-swscale --disable-swresample --disable-avfilter --disable-network \
	--disable-pthreads --disable-w32threads --disable-os2threads \
	--disable-altivec --disable-asm --disable-debug \
	--disable-iconv --disable-zlib --disable-bzlib --disable-lzma \
	--enable-demuxer=bink --enable-decoder=bink --enable-decoder=binkaudio_rdft --enable-decoder=binkaudio_dct \
	--enable-protocol=file --enable-static --disable-shared \
	--extra-cflags="-DESPRESSO -mcpu=750 -meabi -mhard-float -D__WIIU__ -O2 -I$DEVKITPRO/wut/include" \
	> configure.log
make -j"$(nproc)" > make.log
make install > /dev/null
mkdir -p "$DEVKITPRO/portlibs/wiiu/share"
echo "$TAG $COMMIT" > "$DEVKITPRO/portlibs/wiiu/share/ffmpeg-commit.txt"
sha256sum "$DEVKITPRO/portlibs/wiiu/lib/libavformat.a" "$DEVKITPRO/portlibs/wiiu/lib/libavcodec.a" \
	"$DEVKITPRO/portlibs/wiiu/lib/libavutil.a"
