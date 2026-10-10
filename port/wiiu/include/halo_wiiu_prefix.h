/*
HALO_WIIU_PREFIX.H

Force-included ahead of every unit of the Wii U build (-include), in place
of the Linux port's prefix (halo_linux_prefix.h), which it then includes.

The game is built by clang (tools/wiiu_build.py), which only warns where the
prefix's MSVC inline semantics (__inline as static) meet newlib's own static
inline helpers. The map test is built by devkitPPC's gcc
(port/wiiu/maptest/build.sh), which refuses them as `static static`: for it
the C library's headers are read first.
*/

#ifndef __HALO_WIIU_PREFIX_H
#define __HALO_WIIU_PREFIX_H

#ifndef __clang__
#include <sys/lock.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#endif

#include "halo_linux_prefix.h"

#endif // __HALO_WIIU_PREFIX_H
