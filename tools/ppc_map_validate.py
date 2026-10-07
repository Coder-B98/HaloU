#!/usr/bin/env python3

# builds tools/map_validate.c for big-endian 32-bit PowerPC Linux, as
# build/ppc/map_validate: a static program that reads Xbox maps, turns their
# tags around for a big-endian machine (port/linux/game/tag_byte_swap.c) and
# checks them with the tag validator, as the Wii U port will. It runs on any
# Linux (WSL too) under qemu-ppc:
#     qemu-ppc build/ppc/map_validate maps/*.map
#
# needs clang, a PowerPC Linux toolchain (Debian/Ubuntu: gcc-powerpc-linux-gnu
# libc6-dev-powerpc-cross) and `python configure.py --lto=off` run first.

import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from ppc_compile_check import ppc_command  # noqa: E402

OUTPUT_DIR = Path("build/ppc")
GAME_UNITS = ["tag_validate", "cache_file_formats", "tag_byte_swap", "tag_swap_codes"]


def main() -> int:
    subprocess.run(["ninja", "build/linux/halo_msvc_semantics.h"], check=True, stdout=subprocess.DEVNULL)
    commands = subprocess.run(["ninja", "-t", "commands", "linux"], check=True, capture_output=True,
                              text=True).stdout.splitlines()
    sources = [Path("tools/map_validate.c")]
    sources += [Path(f"port/linux/game/{unit}.c") for unit in GAME_UNITS]
    sources += sorted(Path("port/linux/game").glob("tag_schema*.c"))
    sources += sorted(Path("port/third_party/zlib").glob("*.c"))

    objects = []
    for source in sources:
        command = next((c for c in commands if re.search(rf" -c {re.escape(str(source))} -o ", c)), None)
        if command is None:
            continue
        output = OUTPUT_DIR / "obj" / source.with_suffix(".o")
        output.parent.mkdir(parents=True, exist_ok=True)
        args = [a for a in ppc_command(command, output) if not a.startswith("-flto")]
        result = subprocess.run(args, capture_output=True, text=True)
        if result.returncode:
            print(f"{source}:\n{result.stderr}")
            return 1
        objects.append(output)

    program = OUTPUT_DIR / "map_validate"
    subprocess.run(["clang", "--target=powerpc-linux-gnu", "--sysroot=/usr/powerpc-linux-gnu", "-static", "-o", str(program), *map(str, objects)], check=True)
    print(f"built {program}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
