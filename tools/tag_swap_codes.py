#!/usr/bin/env python3

# writes port/linux/game/tag_swap_codes.c: for each structure a tag schema
# names (port/linux/game/tag_schema_*.c, TAG_SCHEMA_DEFINITION), where its
# 2-, 4- and 8-byte integers and reals are, so that a big-endian build can
# turn a map's little-endian tags around (tag_byte_swap.c). The layouts are
# the compiler's (libclang, with the game's own flags for 32-bit big-endian
# PowerPC), so they are exactly the structures the game reads.
#
# needs libclang (pip install libclang), clang, a PowerPC Linux sysroot and
# `python configure.py` run first. Usage: python tools/tag_swap_codes.py
# [--check] (--check fails if the file in the tree is out of date)

import argparse
import re
import shlex
import subprocess
import sys
from pathlib import Path

import clang.cindex as ci

sys.path.insert(0, str(Path(__file__).parent))
from ppc_compile_check import ppc_command  # noqa: E402

OUTPUT = Path("port/linux/game/tag_swap_codes.c")
SCHEMA_SOURCES = sorted(Path("port/linux/game").glob("tag_schema_*.c"))
# an expanded TAG_SCHEMA_DEFINITION, { "name", sizeof(type), (fields), "type" },
# or a definition written out for part of a structure, { "name",
# offsetof(type, field), fields, "type" }
DEFINITION = re.compile(r'\{\s*"\w+"\s*,\s*(?:sizeof|__builtin_offsetof)\s*\([^)]*\)\s*,\s*\(?\s*\w+\s*\)?\s*,'
                        r'\s*"([^"]+)"\s*\}')

K = ci.TypeKind
SCALARS = {
    K.CHAR_S: 1, K.SCHAR: 1, K.CHAR_U: 1, K.UCHAR: 1, K.BOOL: 1,
    K.SHORT: 2, K.USHORT: 2, K.WCHAR: 2, K.CHAR16: 2,
    K.INT: 4, K.UINT: 4, K.LONG: 4, K.ULONG: 4, K.FLOAT: 4,
    K.LONGLONG: 8, K.ULONGLONG: 8, K.DOUBLE: 8,
    K.POINTER: 4,
}

BUILTINS = {"char": 1, "short": 2, "long": 4, "float": 4, "unsigned char": 1, "unsigned short": 2,
            "unsigned long": 4}


class Layout:
    def __init__(self):
        self.runs = []  # [offset, size, count]
        self.problems = []

    def add(self, offset, size, count=1):
        if size == 1:
            return
        last = self.runs[-1] if self.runs else None
        if last and last[1] == size and last[0] + last[1] * last[2] == offset:
            last[2] += count
        else:
            self.runs.append([offset, size, count])

    def walk(self, t, offset, where):
        t = t.get_canonical()
        if t.kind in SCALARS:
            self.add(offset, SCALARS[t.kind])
        elif t.kind == K.ENUM:
            self.walk(t.get_declaration().enum_type, offset, where)
        elif t.kind == K.CONSTANTARRAY:
            element = t.get_array_element_type()
            size = element.get_canonical().get_size()
            for i in range(t.get_array_size()):
                self.walk(element, offset + i * size, f"{where}[{i}]")
        elif t.kind == K.RECORD:
            declaration = t.get_declaration()
            if declaration.kind == ci.CursorKind.UNION_DECL:
                self.union(t, offset, where)
                return
            for field in t.get_fields():
                name = f"{where}.{field.spelling}"
                if field.is_bitfield():
                    self.problems.append(f"bitfield {name}")
                    continue
                self.walk(field.type, offset + field.get_field_offsetof() // 8, name)
        else:
            self.problems.append(f"{t.kind} {where}")

    def union(self, t, offset, where):
        # a union whose members all swap alike is swapped as they are (an
        # `unused` member left aside); otherwise it is reported
        layouts = []
        fields = list(t.get_fields())
        used = [f for f in fields if not f.spelling.startswith("unused")]
        for field in used or fields:
            member = Layout()
            member.walk(field.type, offset, f"{where}.{field.spelling}")
            layouts.append(member)
        if all(m.runs == layouts[0].runs and not m.problems for m in layouts):
            for run in layouts[0].runs:
                self.add(*run)
        else:
            self.problems.append(f"union {where}")


def unit_args(source):
    # the unit's compile, for big-endian PowerPC (ppc_compile_check.py)
    commands = subprocess.run(["ninja", "-t", "commands", "linux"], check=True, capture_output=True,
                              text=True).stdout.splitlines()
    command = next(c for c in commands if f" -c {source} " in c)
    args = ppc_command(command, Path("/dev/null"))[1:]
    return [a for a in args if a not in ("-c", str(source))][:-2]


def schema_types(source, args):
    # the types the unit's schemas name: TAG_SCHEMA_DEFINITION's #type, as
    # the preprocessor writes it (so that definitions made by other macros
    # are found too)
    text = subprocess.run(["clang", "-E", "-P", *args, str(source)], check=True, capture_output=True,
                          text=True).stdout
    return set(DEFINITION.findall(text))


def parse(source, args):
    resource = subprocess.run(["clang", "-print-resource-dir"], check=True, capture_output=True,
                              text=True).stdout.strip()
    tu = ci.Index.create().parse(str(source), args=args + ["-resource-dir", resource])
    errors = [d for d in tu.diagnostics if d.severity >= ci.Diagnostic.Error]
    if errors:
        raise SystemExit(f"{source}: {errors[0]}")
    return tu


def generate():
    types = {}
    for source in SCHEMA_SOURCES:
        args = unit_args(source)
        spellings = schema_types(source, args)
        if not spellings:
            continue
        tu = parse(source, args)
        lookup = {}
        for cursor in tu.cursor.walk_preorder():
            if cursor.kind == ci.CursorKind.STRUCT_DECL and cursor.is_definition():
                lookup.setdefault(f"struct {cursor.spelling}", cursor.type)
            elif cursor.kind == ci.CursorKind.TYPEDEF_DECL:
                lookup.setdefault(cursor.spelling, cursor.type)
        for spelling in spellings:
            layout = Layout()
            if spelling in BUILTINS:
                layout.add(0, BUILTINS[spelling])
                types[spelling] = (BUILTINS[spelling], layout)
                continue
            t = lookup.get(spelling)
            if t is None:
                raise SystemExit(f"{source}: no type {spelling}")
            layout.walk(t, 0, spelling)
            types[spelling] = (t.get_canonical().get_size(), layout)

    lines = [
        "/*",
        "TAG_SWAP_CODES.C",
        "",
        "Written by tools/tag_swap_codes.py: do not edit. Where the integers and",
        "reals are in each structure a tag schema names (tag_byte_swap.c).",
        "*/",
        "",
        '#include "cseries.h"',
        '#include "tag_byte_swap.h"',
        "",
    ]
    problems = []
    for spelling in sorted(types):
        size, layout = types[spelling]
        name = re.sub(r"\W", "_", spelling)
        runs = ", ".join(f"{{{o}, {s}, {c}}}" for o, s, c in layout.runs) or "{0, 0, 0}"
        lines.append(f"static struct tag_swap_run const {name}_runs[] = {{{runs}}};")
        problems += [f"{spelling}: {p}" for p in layout.problems]
    lines += ["", "struct tag_swap_type const tag_swap_types[] =", "{"]
    for spelling in sorted(types):
        size, layout = types[spelling]
        name = re.sub(r"\W", "_", spelling)
        lines.append(f'\t{{"{spelling}", {size}, {len(layout.runs)}, {name}_runs}},')
    lines += ["};", "", f"long const tag_swap_type_count = {len(types)};", ""]
    if problems:
        lines[7:7] = ["/* not swapped (tag_byte_swap.c must handle them itself):"] + \
                     [f"\t{p}" for p in problems] + ["*/", ""]
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    subprocess.run(["ninja", "build/linux/halo_msvc_semantics.h"], check=True, stdout=subprocess.DEVNULL)
    text = generate()
    if args.check:
        if OUTPUT.read_text() != text:
            raise SystemExit(f"{OUTPUT} is out of date: run python tools/tag_swap_codes.py")
        return
    OUTPUT.write_text(text)
    print(f"wrote {OUTPUT}")


if __name__ == "__main__":
    main()
