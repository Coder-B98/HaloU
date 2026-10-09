#!/bin/bash
# builds build/wiiu/halou_maptest.wuhb, the first Wii U test: it reads the
# maps from the SD card (sd:/halou/maps), turns their tags around and checks
# them (maptest.c). Run from the repository's root, after
# `python configure.py`, inside devkitPro's Wii U image:
#     docker run --rm -v "$PWD":/src -w /src devkitpro/devkitppc:latest \
#         bash port/wiiu/maptest/build.sh
set -e
DEVKITPRO=${DEVKITPRO:-/opt/devkitpro}
WUT=$DEVKITPRO/wut
CC=$DEVKITPRO/devkitPPC/bin/powerpc-eabi-gcc
OUT=build/wiiu
mkdir -p $OUT/obj

MACHDEP="-DESPRESSO -mcpu=750 -meabi -mhard-float"
COMMON="$MACHDEP -O2 -g -fsigned-char -fms-extensions -fshort-wchar -fcommon -fno-strict-aliasing -fwrapv
	-fno-delete-null-pointer-checks -D__WIIU__ -D__WUT__ -I$WUT/include"
# the game's units, as the Linux build compiles them (halo_wiiu_prefix.h
# first, for what newlib declares differently)
GAME="$COMMON -w -std=gnu89 -D__STRICT_ANSI__ -DHALO_ANDROID -DDEBUG -Dxbox
	-include port/wiiu/include/halo_wiiu_prefix.h -include build/linux/halo_msvc_semantics.h
	-Iport/linux/include -iquote port/linux/game -Isource -Isource/cseries -Isource/memory
	-Isource/tag_files -Isource/math -idirafter port/include/xdk"
ZLIB="$COMMON -std=gnu11 -Wall -Wno-multichar -DZ_PREFIX -Dz_errmsg=z_port_errmsg -Iport/third_party/zlib"

objects=()
compile() { # flags source
	local object=$OUT/obj/$(basename "${2%.c}").o
	$CC $1 -c "$2" -o "$object"
	objects+=("$object")
}

# (maptest.c is the Wii U side: wut and newlib, without the game's headers)
compile "$ZLIB" port/wiiu/maptest/maptest.c
for unit in tag_validate tag_byte_swap tag_swap_codes; do
	compile "$GAME" port/linux/game/$unit.c
done
for source in port/linux/game/tag_schema*.c; do
	compile "$GAME" "$source"
done
for source in port/third_party/zlib/*.c; do
	compile "$ZLIB" "$source"
done

$CC $MACHDEP -g -specs=$WUT/share/wut.specs -Wl,-Map,$OUT/halou_maptest.map \
	-o $OUT/halou_maptest.elf "${objects[@]}" -L$WUT/lib -lwut
$DEVKITPRO/tools/bin/elf2rpl $OUT/halou_maptest.elf $OUT/halou_maptest.rpx
$DEVKITPRO/tools/bin/wuhbtool $OUT/halou_maptest.rpx $OUT/halou_maptest.wuhb \
	--name="Halo U map test" --short-name="Halo U test" --author="Halo U"
echo "built $OUT/halou_maptest.wuhb"
