#!/usr/bin/env python3

# compiles every game unit of the Linux build for big-endian 32-bit PowerPC
# (the Wii U's byte order and pointer size) and reports which units fail; a
# first check for the Wii U port, which links nothing and runs nothing.
#
# needs clang and a PowerPC Linux sysroot (Debian/Ubuntu:
# gcc-powerpc-linux-gnu libc6-dev-powerpc-cross), and `python configure.py`
# run first. Usage: python tools/ppc_compile_check.py [-j N]

import argparse
import os
import re
import shlex
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

# the Linux build's x86-only flags, and what replaces them for PowerPC
REPLACE = {
    "--target=i686-linux-gnu": ["--target=powerpc-linux-gnu", "-fsigned-char"],
    "-freg-struct-return": ["-msvr4-struct-return"],
}
DROP = {"-m32", "-malign-double", "-march=native", "-march=x86-64"}


def ppc_command(command: str, output: Path) -> list:
    args = shlex.split(command)
    result = []
    skip = 0
    for arg in args:
        if skip:
            skip -= 1
            continue
        if arg in ("-MMD",):
            continue
        if arg in ("-MF", "-o"):
            skip = 1
            continue
        if arg in DROP:
            continue
        result.extend(REPLACE.get(arg, [arg]))
    # halo_linux_prefix.h accepts x86 and the Android guest only
    return result + ["-DHALO_ANDROID", "-o", str(output)]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("-j", type=int, default=os.cpu_count())
    args = parser.parse_args()

    subprocess.run(["ninja", "build/linux/halo_msvc_semantics.h"], check=True, stdout=subprocess.DEVNULL)
    commands = subprocess.run(["ninja", "-t", "commands", "linux"], check=True, capture_output=True,
                              text=True).stdout.splitlines()
    units = [c for c in commands if re.search(r" -c (source|port/linux/game)/", c)]

    with tempfile.TemporaryDirectory() as scratch:
        def compile_unit(index_command):
            index, command = index_command
            source = re.search(r" -c (.+?) -o ", command).group(1)
            done = subprocess.run(ppc_command(command, Path(scratch) / f"{index}.o"), capture_output=True, text=True)
            return source, done.returncode, done.stderr

        with ThreadPoolExecutor(args.j) as pool:
            results = list(pool.map(compile_unit, enumerate(units)))

    failed = [(source, stderr) for source, code, stderr in results if code]
    for source, stderr in failed:
        print(f"=== {source}")
        print("\n".join(line for line in stderr.splitlines() if "error" in line)[:2000])
    print(f"{len(units) - len(failed)} of {len(units)} units compile for big-endian PowerPC")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
