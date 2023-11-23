#pragma once

#define BE_PROFILE

#ifdef BE_PROFILE

#if defined(__clang__) || defined(__GNUC__)
#define TracyFunction __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define TracyFunction __FUNCSIG__
#endif

#include <tracy/Tracy.hpp>

#define PROFILER_MSG(M) TracyMessageL(M)
#define PROFILER_THREAD_NAME(N) tracy::SetThreadName(N)
#define PROFILER_BEGIN_FRAME(N) FrameMarkStart(##N)
#define PROFILER_END_FRAME(N) FrameMarkEnd(##N)
#define PROFILER_SCOPE ZoneScoped
#define PROFILER_SCOPE_N(N) ZoneNamed(N, true)
#define PROFILER_MEM_ALLOC(PTR, SIZE) TracyAlloc(PTR, SIZE)
#define PROFILER_MEM_FREE(PTR) TracyFree(PTR)
#define MUTEX(N) TracyLockable(Mutex, N)

#else

#define PROFILER_MSG(M)
#define PROFILER_THREAD_NAME(N)
#define PROFILER_BEGIN_FRAME(N)
#define PROFILER_END_FRAME(N)
#define PROFILER_SCOPE
#define PROFILER_SCOPE_N(N)
#define MUTEX(N) Mutex N

#endif