# Halo U plan and task list

Goal: Halo: CE (the OpenCE port of the Xbox decompilation) running on the
Wii U: in Cemu first, then on a softmodded console (Aroma, firmware 5.5.6U).
Campaign first. Halo 1 only.

The full assessment, written 2026-10-07, is Claude's project file
`plan/halo-wiiu-gameplan.md`. In short: port the platform layer (graphics,
sound, input, files, network) to the Wii U and keep the game code as it is;
the two big risks were byte order (the Wii U is big-endian, the Xbox and its
maps little-endian) and graphics (no OpenGL on the Wii U, so GX2).

## Milestones

| Tag | Commit | What |
|---|---|---|
| halou-m1-maps-be | fa28c0e | every retail map's tags turned around for big-endian |
| halou-m2-headless | f090656 | whole game builds for the Wii U, nothing drawn |
| halou-m3-menu-loop | ab01a81 | game logic runs in Cemu to the main menu (run r02) |
| halou-m4-first-pixel | (next) | the menu drawn in Cemu |

(Tags to be created once Benjamin agrees; see docs/STATUS.md.)

## Tasks

Each task: ID, goal, done when (checkable), files. "Shared" marks tasks that
change code the PC builds also use (source/, port/linux/, port/include/).

| ID | Task | Done when | Files | State |
|---|---|---|---|---|
| T-001 | Pin the build, embed the build ID, time-stamp the log, write BUILDING-WIIU.md / STATUS.md / PLAN.md | a clean rebuild matches BUILDINFO; halou.log line 1 names the commit | tools/wiiu_build.*, tools/wiiu_cafeglsl.sh, port/wiiu/src/wiiu_main.c, port/linux/src/xbox_kernel.c (shared, Wii U only) | done, awaiting review |
| T-002 | CafeGLSL crash at the first shader compile. Time box: 4 hours of investigation, then precompiled shaders either way | the clear shader compiles in Cemu, or the time box ends and the findings are in STATUS.md | port/wiiu/src/wiiu_gl.c | done 2026-10-11: cause was no __eabi in clang's main, fixed in 35fcaa0, verified by r08 |
| T-003 | Precompiled shaders: dump the generated GLSL from the PC build, compile offline to .gsh, look up by hash, fall back to CafeGLSL with a logged miss. The cache is built from the maps on Benjamin's machine and never committed | menu draws with no runtime compile | port/linux/src/d3d8_gl.c (shared), wiiu_gl.c, tools/ | |
| T-004 | Fix whatever the first picture shows wrong (orientation, colours, textures) | menu looks as on PC | wiiu_gl.c | text: byte order a9eb234, re-upload 310a1b3 (run r13 to test); menu background and orange version text open |
| T-005 | GamePad and Pro Controller input (VPAD, KPAD); fix the save-file open error 0x57 | menus navigable; profile saves | port/wiiu/src/ | input done in 1b9c97c (r11: menus move); save open fix in 310a1b3 (run r13 to test) |
| T-006 | 'metr' tag schema | maptest: 0 groups without a schema | port/linux/game/tag_schema_effects.c (shared) | |
| T-007 | Sound (AX) | menu music audible | port/wiiu/src/ | done: verified by r11 |
| T-008 | Real hardware: measure heap free and map load times first | numbers in a run folder | | |
| T-009 | Merge upstream OpenCE (401 commits behind on 2026-10-11) at a milestone tag, not mid-renderer | one merge commit recording the upstream hash; PC and Wii U builds pass | | |
| T-011 | Bink movies: decode the game's .bik files (intro, attract, credits) with FFmpeg's LGPL Bink decoder, picture into the movie texture, sound into a DirectSound stream. Movies stay on Benjamin's machine (sd:/halou/bink), never committed | intro plays with sound in Cemu | port/wiiu/src/wiiu_bink*.c, tools/wiiu_ffmpeg.sh, tools/wiiu_build.* | decoder verified on BE (qemu) 2026-10-11; player in r16, awaiting run |
| T-012 | Text: letters drawn as 2x2 small copies | menu text reads as on PC | source/rasterizer/rasterizer_text.c (shared) | cause: XBOX_TEXEL16 named its argument twice, so *source++ ran twice; fixed for r16, awaiting run |
| T-013 | Campaign load: a10 scripts loop creating objects, then halt tag_groups.c:3089 | a10 reaches gameplay | source/hs/ (shared) | cause: on BE, script booleans and shorts read from the slot's wrong end, so every `if`/`sleep_until` was false; fixed for r16, awaiting run |
| T-010 | Networking (System Link) | | | last |
