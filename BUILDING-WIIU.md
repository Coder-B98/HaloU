# Building Halo U (the Wii U build)

Everything needed to rebuild `halou.wuhb` from this branch and get the same
bytes. The game data (maps) is never part of a build: it stays on your own
SD card (`sd:/halou/maps`).

## What you need

| What | Version (pinned) | Where it comes from |
|---|---|---|
| Build machine | Ubuntu 24.04 x86-64 (also used for every build so far) | |
| devkitPro (devkitPPC, wut, wut-tools) | image `ghcr.io/wiiu-env/devkitppc:20260504`, digest `sha256:68785e7d75b070b0aea03272b61f36dd2fddfda4f442cdf0e27b168bd36a09fb`. Inside: devkitPPC r50 (gcc 16.1.0), wut 1.9.1, wut-tools 1.3.5 | copied out of the image to `/opt/devkitpro` |
| CafeGLSL (the shader compiler, linked in) | commit `ccf6e1dad2c8be51ea25f9fd4cef8e45221eb000` (reports itself as 0.3.0); `libcafeglsl.a` sha256 `58591ac725311a11764a8853762922e9688ba36fd99d064bcd92fd609b895a10` | built from source by `tools/wiiu_cafeglsl.sh` |
| clang | Ubuntu clang 18.1.3 | `apt install clang` |
| ninja | 1.11.1 | `apt install ninja-build` |
| Python | 3.13 (the version used; older 3.x untested) | |
| CafeGLSL's build tools | bison, flex, python3-venv | `apt install bison flex python3-venv` |

The game is compiled by clang (for its MSVC semantics) and linked by
devkitPPC's gcc with wut: see the header of `tools/wiiu_build.py`.

## Steps

```sh
# 1. devkitPro, from the pinned image (Docker Hub's devkitpro/devkitppc:latest
#    is not pinned; don't use it for a build anyone audits)
docker pull ghcr.io/wiiu-env/devkitppc@sha256:68785e7d75b070b0aea03272b61f36dd2fddfda4f442cdf0e27b168bd36a09fb
id=$(docker create ghcr.io/wiiu-env/devkitppc@sha256:68785e7d75b070b0aea03272b61f36dd2fddfda4f442cdf0e27b168bd36a09fb)
sudo docker cp $id:/opt/devkitpro /opt/ && docker rm $id
sudo chown -R $(id -u) /opt/devkitpro/portlibs/wiiu

# 2. CafeGLSL into /opt/devkitpro/portlibs/wiiu (once)
tools/wiiu_cafeglsl.sh

# 3. the game: build/wiiu/halou.wuhb and build/wiiu/BUILDINFO.txt
tools/wiiu_build.sh
```

`tools/wiiu_build.sh` runs `python3 configure.py --lto off --pgo off`, then
`tools/wiiu_build.py` and ninja. (Link-time optimisation has to be off:
devkitPPC's linker cannot read clang's LTO objects. That flag was missing
from earlier notes.)

## Checking a build

- `build/wiiu/BUILDINFO.txt` records the commit (and whether the tree was
  clean), the toolchain, and the SHA-256 and MD5 of `halou.rpx` and
  `halou.wuhb`. A build from a dirty tree says so and is never released.
- The game writes the same commit as the first line of `sd:/halou/halou.log`
  (`Halo U: build <git describe>`), and every log line starts with the
  seconds since start.
- The build has no date or path inside it that changes between machines, so
  the same commit and toolchain give the same bytes. Checked 2026-10-11:
  rebuilding commit 93682bf this way gave `halou.wuhb` MD5
  `6b4f95e5c80902e5b1bd26054546d7e4`, identical to the build used for run
  r03b on 2026-10-10.

## The other Wii U builds

- `halou_maptest.wuhb` (reads and checks every map on the SD card):
  `port/wiiu/maptest/build.sh` (see its header).
- `map_validate` for big-endian PowerPC Linux, run under qemu-ppc:
  `tools/ppc_map_validate.py`.
- Compile check of every game unit for big-endian PowerPC:
  `tools/ppc_compile_check.py`.

## Notes

- If Docker Hub refuses pulls (rate limits), `mirror.gcr.io/devkitpro/devkitppc`
  mirrors devkitPro's own image, but that image is not the pinned one.
- Process for builds, test runs and audits: `docs/STATUS.md` and the audit SOP
  (Benjamin's copy, `Desktop\Halo\halo-u-audit-sop.docx`).
