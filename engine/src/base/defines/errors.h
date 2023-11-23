#pragma once

// debug break
#ifdef BE_DEBUG
#if defined(BE_COMPILER_MSVC)
#define BE_PRIVATE_BREAK_POINT() __debugbreak()
#elif defined(BE_COMPILER_GCC)
#include <csignal>
#define BE_PRIVATE_BREAK_POINT() std::raise(SIGTRAP)
#endif

#else
#define BE_PRIVATE_BREAK_POINT() \
    {                            \
    }
#endif

// exit
#ifndef BE_PRIVATE_EXIT
#define BE_PRIVATE_EXIT() ::exit(EXIT_FAILURE)
#endif

#ifdef BE_DEBUG
#define FATAL(...)            \
    LOG_ERROR(__VA_ARGS__);   \
    BE_PRIVATE_BREAK_POINT(); \
    BE_PRIVATE_EXIT();

#define VERIFY(expr, ...)         \
    if (!(expr)) [[unlikely]]     \
    {                             \
        LOG_ERROR(__VA_ARGS__);   \
        BE_PRIVATE_BREAK_POINT(); \
        BE_PRIVATE_EXIT();        \
    }

#define ASSERT_MSG(expr, ...)     \
    if (!(expr)) [[unlikely]]     \
    {                             \
        LOG_ERROR(__VA_ARGS__);   \
        BE_PRIVATE_BREAK_POINT(); \
        BE_PRIVATE_EXIT();        \
    }

#define ASSERT(expr)                  \
    if (!(expr)) [[unlikely]]         \
    {                                 \
        LOG_ERROR(BE_TOSTRING(expr)); \
        BE_PRIVATE_BREAK_POINT();     \
        BE_PRIVATE_EXIT();            \
    }

#else
#define FATAL(...)          \
    LOG_ERROR(__VA_ARGS__); \
    BE_PRIVATE_EXIT();

#define VERIFY(expr, ...)       \
    if (!(expr)) [[unlikely]]   \
    {                           \
        LOG_ERROR(__VA_ARGS__); \
        BE_PRIVATE_EXIT();      \
    }

#define ASSERT(...)
#define ASSERT_MSG(expr, ...)
#endif

#define STATIC_ASSERT(/* expr, msg */...) static_assert(BE_GETRAW(BE_GETARG_0(__VA_ARGS__)), BE_GETRAW(BE_GETARG_1(__VA_ARGS__, BE_TOSTRING(__VA_ARGS__))))