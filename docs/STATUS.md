# Halo U status: 2026-10-11

Branch `wiiu` of github.com/Coder-B98/HaloU, forked from OpenCE at 4e8ed2f.
Latest run: r03b (2026-10-10). Milestone: m3, game logic runs, nothing drawn.
Evidence for runs: `C:\Users\Ben\Desktop\Halo\audit\<date>-r<NN>\` on
Benjamin's PC (copies only, never edited). The next run is r04.

Tags: [VERIFIED: pointer] can be checked; [INFERRED] is judgment.
Statuses: VERIFIED, INFERRED, OUTDATED (true before, not now), WITHDRAWN
(was wrong). IDs never change.

## Alerts (claims changed this session)

- A5 WITHDRAWN 2026-10-11: "the game loop ran at 60 fps" was wrong. Run r02
  drew about 30 frames a second (the Xbox rate) for about 5 minutes
  (Grok's audit; audit\2026-10-10-r02\halou.log frame 9001 over >=300 s
  from debug.txt).
- A3 corrected 2026-10-11: the passing qemu evidence is wiiu-test\r7.txt
  (copy in audit\r7.txt), not results.txt (an older run that refused all 24).
- S1 WITHDRAWN 2026-10-11: the +18.7k-line diff is mostly the vendored
  Khronos GLES headers (~9.8k lines), not generated swap tables (664 lines).
- A11 now VERIFIED for r03b: a clean rebuild of 93682bf is byte-identical to
  the r03b build. The r02 build was restored into its run folder.

## Subsystems

| Subsystem | Status | Evidence |
|---|---|---|
| Game code | builds, logic runs | r02 |
| Maps (byte order) | done | r7.txt; maptest.txt (24/24) |
| Graphics (GX2 + CafeGLSL) | built; crashes at the first shader compile | r03b halou.log line 9 |
| Byte order outside the maps (textures, vertices, sound, saves) | untested | |
| Sound, input, network | null | port/wiiu/src/*_null.c |
| Saves / profiles | fail (error 0x57 on open) | r02 debug.txt |
| Build | pinned and reproducible | BUILDING-WIIU.md |

## Claims

- [A1] Source on GitHub, branch wiiu | VERIFIED: git ls-remote (Grok, 2026-10-11)
- [A2] Fork point 4e8ed2f | VERIFIED: git merge-base origin/main origin/wiiu
- [A3] 24/24 maps pass big-endian validation | VERIFIED: audit\r7.txt (0 corrections, 0 fields as read; only 'metr' without a schema)
- [A4] 24/24 also in Cemu | VERIFIED: sdcard\halou\maptest.txt
- [A5] Game loop at 60 fps | WITHDRAWN 2026-10-11, see A5a
- [A5a] r02: main menu reached (logic only), ~30 fps for ~5 min | VERIFIED: audit\2026-10-10-r02 (halou.log, debug.txt)
- [A6] r03b crash is inside CompileShaderPair | VERIFIED: audit\2026-10-10-r03b\halou.log:9; port/wiiu/src/wiiu_gl.c:833-841. Cause: unknown
- [A7] 'metr' has no schema | VERIFIED: port/linux/game/tag_schema_effects.c:2010
- [A8] LARGE_INTEGER fixed for big-endian | VERIFIED: ab01a81; r01 halt gone in r02
- [A9] Shaders compiled on the console by CafeGLSL | VERIFIED: wiiu_gl.c:12, :58, :835
- [A10] No maps in the .wuhb | VERIFIED: Grok, romfs lists only root.rpx
- [A11] Builds trace to commits | VERIFIED for r02 (md5 c166e773…, ab01a81, restored copy) and r03b (md5 6b4f95e5…, = clean rebuild of 93682bf); r01 and r03a builds were not kept
- [A12] BUILDING-WIIU.md reproduces a build's hash | VERIFIED 2026-10-11 by Claude for 93682bf; open for Grok to repeat
- [A13] PC builds unaffected by shared-code commits | open: shared changes since 4e8ed2f have not had a Linux build or test run recorded
- [S7] Profile saves fail with 0x57 | VERIFIED: audit\2026-10-10-r02\debug.txt
- [S8] The clear shader compiles with CafeGLSL on the Wii U side, in Cemu, when the game is not in the program | VERIFIED 2026-10-11: run r04 (audit\2026-10-11-r04\glsltest.txt, build abf896a halou_glsltest.wuhb): main thread, after GX2 start-up and on an 8 MB CPU 1 thread all OK, 88 + 48 bytes
- [S11] All 494 game units compile for PPC | INFERRED: not re-run since 8e1abd4; superseded in practice by the full build linking
- [C1] The CafeGLSL crash is a host-side Cemu crash or memory exhaustion | INFERRED (Grok): no guest exception and no Cemu log line
  - 2026-10-11: "Cemu cannot run CafeGLSL" REFUTED by r04 (the same compiler works in a program of its own). Run r05 (abf896a) still closes Cemu at the first compile with no guest exception line; Cemu does not report bad guest memory accesses as exceptions, so a bad pointer inside the compiler is the likeliest reading [INFERRED]. Memory exhaustion is still open: r05's heap counts read 0 because they asked the system heap, which wut hands to newlib. Build bb5a922 logs newlib's counts and every failed allocation, and compiles the clear program at the start, on the game's thread and in the renderer; run r06 decides which.
  - 2026-10-11: r06 crashed in the compile at the very start of main with ~1 GB free and no failed allocation: memory exhaustion REFUTED. r07a-c (builds\b8cd567): the test app compiling the pair first is fine, linked with all of Halo's objects is fine, compiled by clang crashes. VERIFIED cause (objdump): gcc's main calls __eabi, which runs __init and the C++ constructors; clang's does not, so CafeGLSL's global objects were never built. Halo's main (clang) had no __eabi call. Fixed in 35fcaa0; VERIFIED by r08 (audit\2026-10-11-r08\halou.log: clear and blit programs compiled, "GX2: ready").
- [C2] r08 then crashed in Cemu: guest store to address 8 in __wut_nanosleep (wut 1.9.1 writes the time left even when passed NULL), from d3d8_gl.c's vertical blank thread | VERIFIED: Cemu crash report in audit\2026-10-11-r08\cemu-log.txt, IP 0270bd34 mapped with addr2line on build 35fcaa0. Fixed in 5288583; VERIFIED by r09 (audit\2026-10-11-r09): the game reached the title screen, its first picture through the GX2 renderer (seen by Benjamin; no crash report in cemu-log.txt).

## Open questions for Benjamin

- OK to create the milestone tags m1 to m3 listed in docs/PLAN.md?
- What should the GamePad screen show: a copy of the TV, the map/HUD, or nothing?

## Next tasks

- T-002 CafeGLSL crash, time-boxed, then T-003 precompiled shaders
- T-004 first-picture fixes
- T-005 input and the save fix
- [C3] r11 (96fa346): sound plays (VERIFIED, Benjamin's recording and "AX: sound on" in audit\2026-10-11-r11\halou.log). Text drew as solid blocks: rasterizer_text.c stored its 16-bit texels big-endian, the renderer reads them low byte first. Campaign/Settings crashed at terminal.c:352: a char * used as va_list, which va_start on PowerPC writes through. VERIFIED from the Cemu crash report; both fixed in a9eb234, r12 to confirm. D-pad: Cemu's input mapping points the D-pad at stick axes [INFERRED from controller0.xml].
- [C4] r12 (a9eb234): no crash report; text still blocks; a new profile cannot be made. debug.txt: file_open of u:\UDATA\...\blam.sav and z:\saved\hdmu.map fail with 0x57 (audit\2026-10-11-r12\debug.txt:986-999). VERIFIED cause of 0x57 (wut 1.9.1 devoptab_fsa_open.cpp): the Wii U's open() returns EINVAL for flag mixes with no fopen mode, among them the plain O_WRONLY that file_open (files_windows.c:503, write access, OPEN_EXISTING) produces. Text: the Wii U cannot fault on writes to cached texture pages (wiiu_memory_watch.c), so the letter cache texture, which the game writes through IDirect3DTexture8_LockRect, kept its first upload [INFERRED: fits "blocks", r13 to confirm]. Both fixed in 310a1b3 (r13); that commit's "Task:" line says T-008 by mistake, it is T-004 and T-005.
- [C5] r13 (310a1b3): a profile can be made (VERIFIED: no 0x57 save errors in audit\2026-10-11-r13\debug.txt); D-pad works once remapped in Cemu (VERIFIED, Benjamin). Letters now draw, each cell as a 2x2 of small copies (the letter twice, the next character code twice below) [cause unknown; r14 dumps the letter cache to sd:/halou/textcache*.bin]. Choosing a difficulty halted at bitmaps_inlines.h:244 (debug.txt:1207): shader_environment's specular/reflection fields were reserved bytes in the schema, never swapped (VERIFIED by offsets and the addr2line stack). Fixed in f62fff0 (r14). A layout comparison of every game struct over tag data against the schemas then found smet colours, sky fog and light direction, vehicle AI reals, chicago extra_flags and fog flags (width) wrong on big-endian; fixed in f14d63a (r15). Open: file_delete of missing playlist files reports 0x57 [harmless, INFERRED]; rasterizer_xbox_environment_fog.c:294 reads a chicago map past the tag root [POSSIBLE, little-endian too].
- [C6] r15 (f14d63a): the shader colour halt is gone (VERIFIED, audit\2026-10-11-r15\debug.txt). The first campaign level loads and its scripts run, then halts at tag_groups.c:3089 "#-11 is not a valid index in [#0,#32)" (debug.txt:10323), after a script repeats object_create of 'looking_panel_*' thousands of times [INFERRED cause: script runtime values on big-endian]. Letter-cache dumps from this run are in the audit folder, not yet read.
- [C7] r16 (d35badb, md5 7782aeec…): three fixes, awaiting run. (1) Text 2x2 copies: VERIFIED cause from the r15 letter dumps and rasterizer_text.c: the XBOX_TEXEL16 macro names its argument twice, so `*source++` advanced two pixels per texel on big-endian; fixed in 93bea0c. (2) a10 script loop: VERIFIED cause by reading source/hs: script booleans and shorts sit at the start of their long slot, but `if`, `sleep_until`, `sleep` and others read them with numeric casts, which on big-endian read the slot's other end, so every condition was false; fixed in the hs commit before d35badb [INFERRED that this also clears the tag_groups.c:3089 halt]. (3) Bink movies: FFmpeg n7.1's Bink decoder, built by tools/wiiu_ffmpeg.sh; VERIFIED decoding big-endian under qemu-ppc (intro.bik, 481 frames, correct colours); playback in the game untested.
