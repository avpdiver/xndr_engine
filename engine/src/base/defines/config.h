#pragma once

// mem leak check
#if defined(BE_COMPILER_MSVC) and defined(BE_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#include <malloc.h>
// call at exit
// returns 'true' if no mem leaks
#define BE_DUMP_MEMLEAKS() (::_CrtDumpMemoryLeaks() != 1)
#else
#undef BE_ENABLE_MEMLEAK_CHECKS
#define BE_DUMP_MEMLEAKS() (true)
#endif