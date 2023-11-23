#pragma once

// mark output and input-output function arguments

// OUT for reference - argument is write only
// OUT for pointer   - memory pointed to is write only
#ifndef OUT
#define OUT
#endif

// INOUT for reference - argument is read write
// INOUT for pointer   - memory pointed to is read write
#ifndef INOUT
#define INOUT
#endif

// force inline
#ifndef forceinline
#if defined(BE_COMPILER_GCC)
#define forceinline inline
#elif defined(BE_COMPILER_MSVC)
#define forceinline __forceinline
#else
#pragma warning("'forceinline' is not supported")
#define forceinline inline
#endif
#endif

// thiscall, cdecl
#ifdef BE_COMPILER_MSVC
#define BE_CDECL __cdecl
#define BE_THISCALL __thiscall
#elif defined(BE_COMPILER_GCC)
#define BE_CDECL    //__attribute__((cdecl))
#define BE_THISCALL //__attribute__((thiscall))
#endif

// DLL import/export
#if not defined(BE_DLL_EXPORT) or !defined(BE_DLL_IMPORT)
#if defined(BE_COMPILER_MSVC)
#define BE_DLL_EXPORT __declspec(dllexport)
#define BE_DLL_IMPORT __declspec(dllimport)
#elif defined(BE_COMPILER_GCC)
#ifdef BE_PLATFORM_WINDOWS
#define BE_DLL_EXPORT __attribute__(dllexport)
#define BE_DLL_IMPORT __attribute__(dllimport)
#else
#define BE_DLL_EXPORT __attribute__((visibility("default")))
#define BE_DLL_IMPORT __attribute__((visibility("default")))
#endif
#else
#error define BE_DLL_EXPORT and BE_DLL_IMPORT for you compiler
#endif
#endif
