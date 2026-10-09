/*
HALO_WIIU_PREFIX.H

Force-included ahead of every game unit of the Wii U build (devkitPPC's gcc,
-include), before the Linux port's prefix (halo_linux_prefix.h), which it
then includes: the C library's headers are read first, so that the prefix's
MSVC inline semantics (__inline as static) cannot reach newlib's own static
inline helpers, which gcc refuses as `static static`.
*/

#ifndef __HALO_WIIU_PREFIX_H
#define __HALO_WIIU_PREFIX_H

#include <sys/lock.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#include "halo_linux_prefix.h"

#endif // __HALO_WIIU_PREFIX_H
