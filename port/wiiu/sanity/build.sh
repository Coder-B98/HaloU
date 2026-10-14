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
$CC $MACHDEP -O2 -g -Wall -D__WIIU__ -D__WUT__ -I$WUT/include \
	-DHALOU_BUILD_ID="\"$(git describe --always --dirty --abbrev=12)\"" \
	-c port/wiiu/sanity/sanity.c -o $OUT/obj/sanity.o
$CC $MACHDEP -g -specs=$WUT/share/wut.specs -o $OUT/halou_sanity.elf $OUT/obj/sanity.o -L$WUT/lib -lwut -lm
$DEVKITPRO/tools/bin/elf2rpl $OUT/halou_sanity.elf $OUT/halou_sanity.rpx
$DEVKITPRO/tools/bin/wuhbtool $OUT/halou_sanity.rpx $OUT/halou_sanity.wuhb \
	--name="Halo U console test" --short-name="Halo U test" --author="Halo U"
md5sum $OUT/halou_sanity.wuhb
