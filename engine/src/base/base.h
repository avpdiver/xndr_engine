#pragma once

#if defined(BE_PLATFORM_WINDOWS)
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef NOMCX
#define NOMCX
#endif

#ifndef NOIME
#define NOIME
#endif

#ifndef NOSERVICE
#define NOSERVICE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef BE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4668)
#pragma warning(disable : 4005)
#pragma warning(pop)
#endif

#include <windows.h>
#else
#include <pthread.h>
#endif

#include "base/profiler/profiler.h"
#include "base/types/std_includes.h"
#include "base/log/log.h"
#include "base/defines/defines.h"
#include "base/types/types.h"
#include "base/platform/platform.h"
#include "base/math/math.h"
#include "base/threading/thread_utils.h"
#include "base/pointers/pointers.h"
#include "base/memory/memory.h"
#include "base/allocators/allocators.h"
#include "base/containers/containers.h"
#include "base/devices/device.h"
#include "base/window/window.h"
#include "base/io/io.h"
#include "base/assets/assets.h"