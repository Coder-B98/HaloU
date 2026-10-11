#!/bin/bash
# T-002: the shader test built in the ways Halo's own build differs from it,
# so one round of runs in Cemu says which difference makes the compiler crash
# (Halo crashes in its first compile even at the very start of main, run r06;
# the plain test does not, run r04). Run from the repository's root after
# tools/wiiu_build.sh (variant 3 links Halo's own objects). Each app writes
# to sd:/halou/glsltest.txt, its first line naming the variant.
#   1 halou_glsltest_pair.wuhb   the plain test, compiling the pair first as Halo does
#   2 halou_glsltest_clang.wuhb  the test compiled by clang with wiiu_gl.c's flags
#   3 halou_glsltest_halo.wuhb   the test linked with all of Halo's objects (not wiiu_main.o)
set -e
export DEVKITPRO=${DEVKITPRO:-/opt/devkitpro}
WUT=$DEVKITPRO/wut
GCC=$DEVKITPRO/devkitPPC/bin/powerpc-eabi-gcc
OUT=build/wiiu
mkdir -p $OUT/obj/glsltest
echo "#define HALOU_BUILD_ID \"$(git describe --always --dirty --abbrev=12)\"" > $OUT/glsltest_build_id.h
MACHDEP="-DESPRESSO -mcpu=750 -meabi -mhard-float"
INCLUDES="-D__WIIU__ -D__WUT__ -I$WUT/include -I$DEVKITPRO/portlibs/wiiu/include -include $OUT/glsltest_build_id.h"
LIBS="-L$DEVKITPRO/portlibs/wiiu/lib -L$WUT/lib -lcafeglsl -lstdc++ -lwut -lm"

package() {
	$DEVKITPRO/devkitPPC/bin/powerpc-eabi-objcopy --remove-section .note.GNU-stack $OUT/$1.elf $OUT/$1.strip.elf
	$DEVKITPRO/tools/bin/elf2rpl $OUT/$1.strip.elf $OUT/$1.rpx
	$DEVKITPRO/tools/bin/wuhbtool $OUT/$1.rpx $OUT/$1.wuhb \
		--name="Halo U shader test $2" --short-name="Halo U shaders" --author="Halo U"
	md5sum $OUT/$1.wuhb
}

# 1: gcc, the pair first
$GCC $MACHDEP -O2 -g -Wall $INCLUDES -DGLSLTEST_VARIANT='"pair first"' -DGLSLTEST_PAIR_FIRST \
	-c port/wiiu/glsltest/glsltest.c -o $OUT/obj/glsltest/pair.o
$GCC $MACHDEP -g -specs=$WUT/share/wut.specs -o $OUT/halou_glsltest_pair.elf $OUT/obj/glsltest/pair.o $LIBS
package halou_glsltest_pair 1

# 2: clang, wiiu_gl.c's flags (from build.ninja), the pair first
CLANG_FLAGS=$(python3 - <<'PY'
import re
text = open("build/wiiu/build.ninja").read().replace("$\n", "")
match = re.search(r"build build/wiiu/obj/port/wiiu/src/wiiu_gl.o: cc port/wiiu/src/wiiu_gl.c\n\s+flags = (.*)", text)
print(" ".join(match.group(1).split()))
PY
)
clang $CLANG_FLAGS -include $OUT/glsltest_build_id.h -DGLSLTEST_VARIANT='"clang"' -DGLSLTEST_PAIR_FIRST \
	-c port/wiiu/glsltest/glsltest.c -o $OUT/obj/glsltest/clang.o
$GCC $MACHDEP -g -specs=$WUT/share/wut.specs -o $OUT/halou_glsltest_clang.elf $OUT/obj/glsltest/clang.o $LIBS
package halou_glsltest_clang 2

# 3: gcc, the pair first, with Halo's objects and Halo's link line
$GCC $MACHDEP -O2 -g -Wall $INCLUDES -DGLSLTEST_VARIANT='"in Halo"' -DGLSLTEST_PAIR_FIRST \
	-c port/wiiu/glsltest/glsltest.c -o $OUT/obj/glsltest/halo.o
printf 'unsigned long wiiu_log_milliseconds(void) { return 0; }\n' > $OUT/obj/glsltest/shim.c
$GCC $MACHDEP -O2 -c $OUT/obj/glsltest/shim.c -o $OUT/obj/glsltest/shim.o
# (Halo's objects as its link takes them; some paths have spaces)
find $OUT/obj -name '*.o' -not -path '*/glsltest/*' -not -name wiiu_main.o -not -name glsltest.o | sort \
	| sed 's/.*/"&"/' > $OUT/obj/glsltest/halo_objects.txt
$GCC $MACHDEP -g -specs=$WUT/share/wut.specs -Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=memalign \
	-o $OUT/halou_glsltest_halo.elf $OUT/obj/glsltest/halo.o $OUT/obj/glsltest/shim.o \
	@$OUT/obj/glsltest/halo_objects.txt $LIBS
package halou_glsltest_halo 3
