#!/bin/bash
# builds build/wiiu/halou_glsltest.wuhb, the Wii U test of CafeGLSL alone
# (glsltest.c). Run from the repository's root with devkitPro and CafeGLSL
# installed as BUILDING-WIIU.md says.
set -e
export DEVKITPRO=${DEVKITPRO:-/opt/devkitpro}
WUT=$DEVKITPRO/wut
CC=$DEVKITPRO/devkitPPC/bin/powerpc-eabi-gcc
OUT=build/wiiu
mkdir -p $OUT/obj
echo "#define HALOU_BUILD_ID \"$(git describe --always --dirty --abbrev=12)\"" > $OUT/glsltest_build_id.h
MACHDEP="-DESPRESSO -mcpu=750 -meabi -mhard-float"
$CC $MACHDEP -O2 -g -Wall -D__WIIU__ -D__WUT__ -I$WUT/include -I$DEVKITPRO/portlibs/wiiu/include \
	-include $OUT/glsltest_build_id.h -Iport/wiiu/glsltest \
	-c port/wiiu/glsltest/glsltest.c -o $OUT/obj/glsltest.o
$CC $MACHDEP -g -specs=$WUT/share/wut.specs -o $OUT/halou_glsltest.elf $OUT/obj/glsltest.o \
	-L$DEVKITPRO/portlibs/wiiu/lib -L$WUT/lib -lcafeglsl -lstdc++ -lwut -lm
$DEVKITPRO/tools/bin/elf2rpl $OUT/halou_glsltest.elf $OUT/halou_glsltest.rpx
$DEVKITPRO/tools/bin/wuhbtool $OUT/halou_glsltest.rpx $OUT/halou_glsltest.wuhb \
	--name="Halo U shader test" --short-name="Halo U shaders" --author="Halo U"
md5sum $OUT/halou_glsltest.wuhb
