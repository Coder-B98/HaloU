#!/bin/bash
# builds CafeGLSL (github.com/Exzap/CafeGLSL), the shader compiler the Wii U
# build links (-lcafeglsl), at the commit BUILDING-WIIU.md pins, and
# installs it into devkitPro's Wii U portlibs. The same steps as CafeGLSL's
# own Dockerfile.wiiu, run on the build machine (needs bison, flex, ninja,
# python3 with venv; devkitPro in $DEVKITPRO). Usage:
#     tools/wiiu_cafeglsl.sh [work folder]
set -e
COMMIT=ccf6e1dad2c8be51ea25f9fd4cef8e45221eb000
DEVKITPRO=${DEVKITPRO:-/opt/devkitpro}
WORK=${1:-build/cafeglsl}
mkdir -p "$WORK"
WORK=$(cd "$WORK" && pwd)

if [ ! -d "$WORK/src/.git" ]; then
	git clone -q https://github.com/Exzap/CafeGLSL.git "$WORK/src"
fi
git -C "$WORK/src" checkout -q $COMMIT
if [ ! -x "$WORK/venv/bin/meson" ]; then
	python3 -m venv "$WORK/venv"
	"$WORK/venv/bin/pip" install -q 'meson>=1.4' mako packaging pyyaml
fi

export PATH="$WORK/venv/bin:$DEVKITPRO/devkitPPC/bin:$DEVKITPRO/portlibs/wiiu/bin:$DEVKITPRO/portlibs/ppc/bin:$PATH"
export DEVKITPRO DEVKITPPC=$DEVKITPRO/devkitPPC
cd "$WORK/src"
rm -rf build-cafe
./cafecompiler/meson-cross.sh wiiu ppccross-container build-cafe \
	-Dtools= -Dgallium-drivers=r600 -Dvulkan-drivers= -Dglx=disabled -Degl=disabled -Dplatforms= \
	-Dllvm=disabled -Db_sanitize=none -Db_lundef=false -Db_staticpic=false -Db_lto=false --buildtype=release
ninja -C build-cafe cafecompiler/libcafeglsl.a
./cafecompiler/ci/package.sh wiiu build-cafe "$DEVKITPRO/portlibs/wiiu"
mkdir -p "$DEVKITPRO/portlibs/wiiu/share"
echo "$COMMIT" > "$DEVKITPRO/portlibs/wiiu/share/cafeglsl-commit.txt"
sha256sum "$DEVKITPRO/portlibs/wiiu/lib/libcafeglsl.a"
