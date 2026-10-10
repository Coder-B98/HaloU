#!/usr/bin/env python3

# writes build/wiiu/build.ninja, the Wii U build of the game: the Linux
# build's own compile commands (`ninja -t commands linux`), turned from
# clang for 32-bit x86 into clang for the Wii U's Espresso (a big-endian
# 32-bit PowerPC, the embedded ABI devkitPPC's gcc has), with wut's headers
# and devkitPPC's newlib in place of glibc; devkitPPC's gcc links them with
# wut. The Wii U's platform layer is port/wiiu/src, with those of the Linux
# layer's units (port/linux/src) that need nothing the Wii U lacks.
#
# The game is compiled by clang, as on Linux, rather than by devkitPPC's
# gcc: its MSVC semantics (halo_linux_prefix.h) are clang's, and the two
# compilers call functions and lay out data the same way on the Espresso
# (but for long double, made 64-bit as gcc has it).
#
# needs clang and devkitPro's devkitPPC and wut in /opt/devkitpro (copied
# out of the devkitpro/devkitppc Docker image: docker cp), and
# `python configure.py` run first. From the repository's root:
#     python tools/wiiu_build.py && ninja -f build/wiiu/build.ninja

import re
import shlex
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from ninja_syntax import Writer  # noqa: E402

BUILD = Path("build/wiiu")
OBJ = BUILD / "obj"
WIIU_DIR = Path("port/wiiu")
DEVKITPRO = "/opt/devkitpro"
WUT = f"{DEVKITPRO}/wut"
CC = "clang"
LINK = f"{DEVKITPRO}/devkitPPC/bin/powerpc-eabi-gcc"

# the Espresso (as devkitPPC's gcc: MACHDEP), and wut's and newlib's headers
MACHDEP = ["-DESPRESSO", "-mcpu=750", "-meabi", "-mhard-float"]
WIIU_FLAGS = ["--target=powerpc-unknown-eabi", "-mcpu=750", "-mhard-float", "-mlong-double-64",
              "-DESPRESSO", "-D__WIIU__", "-D__WUT__", "-fsigned-char",
              f"-isystem{WUT}/include", f"-isystem{DEVKITPRO}/devkitPPC/powerpc-eabi/include"]

# the Linux layer's units the Wii U builds as they are (the others need
# SDL3, OpenGL, the network or the self-updater: the Wii U has its own, or
# none yet)
LINUX_PLATFORM_UNITS = [
    "bink_null.c",
    "xbdm.c",
    "d3d8_resources.c",
    "halo_linker_common.c",
    "msvc_crt.c",
    "msvc_wide.c",
    "port_config.c",
    "posix_files.c",
    "xbox_files.c",
    "xbox_kernel.c",
    "xbox_texture_layout.c",
    "xbox_xapi.c",
]

# the Wii U's units that are not the Xbox's platform layer, built without it
WIIU_STARTUP_UNITS = ["wiiu_main.c"]

# the Linux build's flags with no meaning for gcc or the Wii U
DROP = {
    "-m32", "-malign-double", "-march=native", "-march=x86-64", "-freg-struct-return", "-fno-pic",
    "-fstack-protector-strong", "-D_FILE_OFFSET_BITS=64",
}
DROP_PREFIXES = ("--target=", "-Wno-microsoft", "-Wno-pragma-pack", "-Wno-ignored-pragmas",
                 "-Wno-error=incompatible-function-pointer-types", "-Wno-duplicate-decl-specifier")


def wiiu_flags(command: str):
    """the source and the gcc flags of one of the Linux build's compile
    commands"""
    args = shlex.split(command)[1:]
    flags = []
    source = None
    skip = 0
    for index, arg in enumerate(args):
        if skip:
            skip -= 1
            continue
        if arg in ("-MMD",):
            continue
        if arg in ("-MF", "-o"):
            skip = 1
            continue
        if arg == "-c":
            source = args[index + 1]
            skip = 1
            continue
        if arg in DROP or arg.startswith(DROP_PREFIXES):
            continue
        # (the Linux build's generated headers, for the Wii U build's own)
        arg = arg.replace("build/linux/", "build/wiiu/")
        if arg == "port/linux/include/halo_linux_prefix.h":
            arg = "port/wiiu/include/halo_wiiu_prefix.h"
        flags.append(arg)
    return source, WIIU_FLAGS + flags


def main() -> int:
    subprocess.run(["ninja", "build/linux/halo_msvc_semantics.h", "build/linux/platform_msvc_semantics.h"],
                   check=True, stdout=subprocess.DEVNULL)
    commands = subprocess.run(["ninja", "-t", "commands", "linux"], check=True, capture_output=True,
                              text=True).stdout.splitlines()
    BUILD.mkdir(parents=True, exist_ok=True)
    for header in ("halo_msvc_semantics.h", "platform_msvc_semantics.h"):
        (BUILD / header).write_text((Path("build/linux") / header).read_text())

    units = []
    for command in commands:
        match = re.search(r" -c (.+?) -o ", command)
        if not match or not command.startswith("clang"):
            continue
        source = match.group(1).strip('"\'')
        if source.startswith(("source/", "port/linux/game/", "port/third_party/musl-math/",
                              "port/third_party/zlib/", "port/third_party/tomlc17/")):
            source, flags = wiiu_flags(command)
            # (the game's main runs after the Wii U's own start-up: port/wiiu/src/wiiu_main.c)
            if source == "source/shell/shell_xbox.c":
                flags = flags + ["-Dmain=halo_xbox_main"]
            units.append((source, flags))
        elif source.startswith("port/linux/src/") and Path(source).name in LINUX_PLATFORM_UNITS:
            source, flags = wiiu_flags(command)
            # (the platform layer's own Wii U headers ahead of the Linux layer's)
            units.append((source, ["-Iport/wiiu/src", "-Iport/wiiu/include"] + flags))
    # the Wii U's own platform units, as the Linux layer's are built
    platform = next(wiiu_flags(c)[1] for c in commands if " -c port/linux/src/xbox_kernel.c " in c)
    for source in sorted((WIIU_DIR / "src").glob("*.c")):
        if source.name in WIIU_STARTUP_UNITS:
            # (the Wii U's own C, with wut's headers, not the Xbox's)
            units.append((str(source), WIIU_FLAGS + ["-O2", "-Wall"]))
        else:
            units.append((str(source), ["-Iport/wiiu/src", "-Iport/wiiu/include"] + platform))

    with open(BUILD / "build.ninja", "w") as file:
        n = Writer(file)
        n.variable("cc", CC)
        n.variable("link", LINK)
        n.rule("cc", f"$cc -MMD -MF $out.d $flags -c $in -o $out", depfile="$out.d", deps="gcc",
               description="WIIU CC $out")
        n.rule("link", f"DEVKITPRO={DEVKITPRO} $link {' '.join(MACHDEP)} -g -specs={WUT}/share/wut.specs -Wl,-Map,$out.map"
               f" -o $out @$out.rsp -L{WUT}/lib -lwut -lm",
               rspfile="$out.rsp", rspfile_content="$in_newline", description="WIIU LINK $out")
        # (clang marks each object's stack not executable in a .note.GNU-stack
        # section, which elf2rpl has no place for; the Wii U's never is)
        n.rule("rpx", f"{DEVKITPRO}/devkitPPC/bin/powerpc-eabi-objcopy --remove-section .note.GNU-stack $in $out.elf"
               f" && {DEVKITPRO}/tools/bin/elf2rpl $out.elf $out", description="WIIU RPX $out")
        n.rule("wuhb", f"{DEVKITPRO}/tools/bin/wuhbtool $in $out --name=\"Halo U\" --short-name=\"Halo U\""
               " --author=\"Halo U\"", description="WIIU WUHB $out")
        objects = []
        for source, flags in units:
            obj = OBJ / Path(source).with_suffix(".o")
            objects.append(obj)
            n.build(str(obj), "cc", source,
                    variables={"flags": " ".join(shlex.quote(flag) for flag in flags)})
        n.build(str(BUILD / "halou.elf"), "link", [str(o) for o in objects])
        n.build(str(BUILD / "halou.rpx"), "rpx", str(BUILD / "halou.elf"))
        n.build(str(BUILD / "halou.wuhb"), "wuhb", str(BUILD / "halou.rpx"))
        n.default(str(BUILD / "halou.wuhb"))
    print(f"wrote {BUILD / 'build.ninja'}: {len(units)} units")
    return 0


if __name__ == "__main__":
    sys.exit(main())
