#!/bin/bash
# builds build/wiiu/halou_sanity.wuhb, the Wii U test of the console itself
# (sanity.c: colours on both screens and a tone, the plain WHBGfx way). Run
# from the repository's root with devkitPro installed as BUILDING-WIIU.md says.
set -e
export DEVKITPRO=${DEVKITPRO:-/opt/devkitpro}
WUT=$DEVKITPRO/wut
CC=$DEVKITPRO/devkitPPC/bin/powerpc-eabi-gcc
OUT=build/wiiu
mkdir -p $OUT/obj
MACHDEP="-DESPRESSO -mcpu=750 -meabi -mhard-float"
for variant in "" _thread; do
	flags=""
	[ "$variant" = _thread ] && flags="-DSANITY_ON_THREAD"
	$CC $MACHDEP -O2 -g -Wall -D__WIIU__ -D__WUT__ -I$WUT/include $flags \
		-DHALOU_BUILD_ID="\"$(git describe --always --dirty --abbrev=12)\"" \
		-c port/wiiu/sanity/sanity.c -o $OUT/obj/sanity$variant.o
	$CC $MACHDEP -g -specs=$WUT/share/wut.specs -o $OUT/halou_sanity$variant.elf $OUT/obj/sanity$variant.o \
		-L$WUT/lib -lwut -lm
	$DEVKITPRO/tools/bin/elf2rpl $OUT/halou_sanity$variant.elf $OUT/halou_sanity$variant.rpx
	name="Halo U console test"
	[ "$variant" = _thread ] && name="Halo U console test 2"
	$DEVKITPRO/tools/bin/wuhbtool $OUT/halou_sanity$variant.rpx $OUT/halou_sanity$variant.wuhb \
		--name="$name" --short-name="$name" --author="Halo U"
	md5sum $OUT/halou_sanity$variant.wuhb
done
