#!/bin/bash
# builds the Wii U game, build/wiiu/halou.wuhb, and writes
# build/wiiu/BUILDINFO.txt: the commit, the toolchain and the hashes, so a
# delivered .wuhb can be traced to what made it (BUILDING-WIIU.md). Run from
# the repository's root:
#     tools/wiiu_build.sh
set -e
DEVKITPRO=${DEVKITPRO:-/opt/devkitpro}
OUT=build/wiiu

# no link-time or profile-guided optimisation: devkitPPC's linker cannot
# read clang's LTO bitcode, and the profiles are the desktop's
python3 configure.py --lto off --pgo off >/dev/null
python3 tools/wiiu_build.py
ninja -f $OUT/build.ninja 2>&1 | tee $OUT/build.log | tail -1

pacman_version() {
	ls "$DEVKITPRO/pacman/var/lib/pacman/local" 2>/dev/null | grep -E "^$1-[0-9r]" | head -1
}
{
	echo "commit      $(git rev-parse HEAD) ($(git diff --quiet HEAD && echo clean || echo DIRTY: never release))"
	echo "describe    $(sed -n 's/.*"\(.*\)".*/\1/p' $OUT/halou_build_id.h)"
	echo "built       $(date -u +%Y-%m-%dT%H:%M:%SZ)"
	echo "devkitPPC   $($DEVKITPRO/devkitPPC/bin/powerpc-eabi-gcc --version | head -1); $(pacman_version devkitPPC)"
	echo "wut         $(pacman_version wut); $(pacman_version wut-tools)"
	echo "devkitPro   ${DEVKITPRO_IMAGE:-ghcr.io/wiiu-env/devkitppc@sha256:68785e7d75b070b0aea03272b61f36dd2fddfda4f442cdf0e27b168bd36a09fb (as BUILDING-WIIU.md pins; set DEVKITPRO_IMAGE if not)}"
	echo "clang       $(clang --version | head -1)"
	echo "CafeGLSL    $(cat $DEVKITPRO/portlibs/wiiu/share/cafeglsl-commit.txt 2>/dev/null || echo 'unknown: not built by tools/wiiu_cafeglsl.sh')"
	echo "python      $(python3 --version)"
	echo "halou.rpx   sha256 $(sha256sum $OUT/halou.rpx | cut -d' ' -f1)"
	echo "halou.wuhb  sha256 $(sha256sum $OUT/halou.wuhb | cut -d' ' -f1)  md5 $(md5sum $OUT/halou.wuhb | cut -d' ' -f1)  size $(stat -c %s $OUT/halou.wuhb)"
	echo "shared code changed since the last BUILDINFO: see the commit's \"Shared code:\" line"
} > $OUT/BUILDINFO.txt
cat $OUT/BUILDINFO.txt
