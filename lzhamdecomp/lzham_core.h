// File: lzham_core.h
// LZHAM is in the Public Domain. Please see the Public Domain declaration at the end of include/lzham.h
#pragma once

#if defined(_MSC_VER)
   #pragma warning (disable: 4127) // conditional expression is constant
#endif

// Enable this when first porting to new platforms - disables all threading and atomic ops in compressor:
//#define LZHAM_ANSI_CPLUSPLUS 1

#if defined(__FreeBSD__) || defined(__NetBSD__)
   // TODO: I haven't compiled/tested on these platforms yet, let's play it safe for now.
   #define LZHAM_ANSI_CPLUSPLUS 1
#endif

#if defined(__APPLE__) && defined(__MACH__)
   // Apple OSX and iOS
   #include <TargetConditionals.h>
#endif

#if defined(_XBOX) && !defined(LZHAM_ANSI_CPLUSPLUS)
   // --- X360 - This hasn't been tested since early an alpha.
   #include <xtl.h>
   #define _HAS_EXCEPTIONS 0
   #define NOMINMAX

   #define LZHAM_PLATFORM_X360 1
   #define LZHAM_USE_WIN32_API 1
   #define LZHAM_USE_WIN32_ATOMIC_FUNCTIONS 1
   #define LZHAM_64BIT_POINTERS 0
   #define LZHAM_CPU_HAS_64BIT_REGISTERS 1
   #define LZHAM_BIG_ENDIAN_CPU 1
   #define LZHAM_USE_UNALIGNED_INT_LOADS 1
   #define LZHAM_RESTRICT __restrict
   #define LZHAM_FORCE_INLINE __forceinline
   #define LZHAM_NOTE_UNUSED(x) (void)x

#elif defined(WIN32) && !defined(LZHAM_ANSI_CPLUSPLUS)
   // --- Windows: MSVC or MinGW, x86 or x64, Win32 API's for threading and Win32 Interlocked API's or GCC built-ins for atomic ops.
   #ifdef NDEBUG
      // Ensure checked iterators are disabled.
      #define _SECURE_SCL 0
      #define _HAS_ITERATOR_DEBUGGING 0
   #endif
   #ifndef _DLL
      // If we're using the DLL form of the run-time libs, we're also going to be enabling exceptions because we'll be building CLR apps.
      // Otherwise, we disable exceptions for a small speed boost.
      #define _HAS_EXCEPTIONS 0
   #endif
   #define NOMINMAX

    #ifndef _WIN32_WINNT
      #define _WIN32_WINNT 0x500
   #endif

   #ifndef WIN32_LEAN_AND_MEAN
      #define WIN32_LEAN_AND_MEAN
   #endif

   #include <windows.h>

   #define LZHAM_USE_WIN32_API 1

   #if defined(__MINGW32__) || defined(__MINGW64__)
      #define LZHAM_USE_GCC_ATOMIC_BUILTINS 1
   #else
      #define LZHAM_USE_WIN32_ATOMIC_FUNCTIONS 1
   #endif

   #define LZHAM_PLATFORM_PC 1

   #ifdef _WIN64
      #define LZHAM_PLATFORM_PC_X64 1
      #define LZHAM_64BIT_POINTERS 1
      #define LZHAM_CPU_HAS_64BIT_REGISTERS 1
      #define LZHAM_LITTLE_ENDIAN_CPU 1
   #else
      #define LZHAM_PLATFORM_PC_X86 1
      #define LZHAM_64BIT_POINTERS 0
      #define LZHAM_CPU_HAS_64BIT_REGISTERS 0
      #define LZHAM_LITTLE_ENDIAN_CPU 1
   #endif

   #define LZHAM_USE_UNALIGNED_INT_LOADS 1
   #define LZHAM_RESTRICT __restrict
   #define LZHAM_FORCE_INLINE __forceinline

   #if (defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__))  && (!(defined(PREVENT_LZHAM_USE_INTRINSIC)))
      #define LZHAM_USE_MSVC_INTRINSICS 1
   #endif

   #define LZHAM_NOTE_UNUSED(x) (void)x

#elif defined(__APPLE__) && !defined(LZHAM_ANSI_CPLUSPLUS)
   // --- Apple: iOS or OSX
   #if (TARGET_IPHONE_SIMULATOR == 1) || (TARGET_OS_IPHONE == 1)
      #define LZHAM_PLATFORM_PC 0

      #if defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__)
         #define LZHAM_PLATFORM_PC_X64 0
         #define LZHAM_64BIT_POINTERS 1
         #define LZHAM_CPU_HAS_64BIT_REGISTERS 1
      #else
         #define LZHAM_PLATFORM_PC_X86 0
         #define LZHAM_64BIT_POINTERS 0
         #define LZHAM_CPU_HAS_64BIT_REGISTERS 0
      #endif

      #define LZHAM_USE_UNALIGNED_INT_LOADS 0

      #if __BIG_ENDIAN__
         #define LZHAM_BIG_ENDIAN_CPU 1
      #else
         #define LZHAM_LITTLE_ENDIAN_CPU 1
      #endif

      #define LZHAM_USE_PTHREADS_API 1
      #define LZHAM_USE_GCC_ATOMIC_BUILTINS 1

      #define LZHAM_RESTRICT

      #if defined(__clang__)
         #define LZHAM_FORCE_INLINE inline
      #else
         #define LZHAM_FORCE_INLINE inline __attribute__((__always_inline__,__gnu_inline__))
      #endif

      #define LZHAM_NOTE_UNUSED(x) (void)x

   #elif (TARGET_OS_MAC == 1)
      #define LZHAM_PLATFORM_PC 1

      #if defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__)
         #define LZHAM_PLATFORM_PC_X64 1
         #define LZHAM_64BIT_POINTERS 1
         #define LZHAM_CPU_HAS_64BIT_REGISTERS 1
      #else
         #define LZHAM_PLATFORM_PC_X86 1
         #define LZHAM_64BIT_POINTERS 0
         #define LZHAM_CPU_HAS_64BIT_REGISTERS 0
      #endif

      #define LZHAM_USE_UNALIGNED_INT_LOADS 1

      #if __BIG_ENDIAN__
         #define LZHAM_BIG_ENDIAN_CPU 1
      #else
         #define LZHAM_LITTLE_ENDIAN_CPU 1
      #endif

      #define LZHAM_USE_PTHREADS_API 1
      #define LZHAM_USE_GCC_ATOMIC_BUILTINS 1

      #define LZHAM_RESTRICT

      #if defined(__clang__)
         #define LZHAM_FORCE_INLINE inline
      #else
         #define LZHAM_FORCE_INLINE inline __attribute__((__always_inline__,__gnu_inline__))
      #endif

      #define LZHAM_NOTE_UNUSED(x) (void)x
   #elif
      #error TODO: Unknown Apple target
   #endif

#elif defined(__linux__) && !defined(ANDROID) && (defined(__i386__) || defined(__x86_64__)) && !defined(LZHAM_ANSI_CPLUSPLUS) 
   // --- Generic GCC/clang path for x86/x64, clang or GCC, Linux, OSX, FreeBSD or NetBSD, pthreads for threading, GCC built-ins for atomic ops.
   #define LZHAM_PLATFORM_PC 1

   #if defined(_LP64) || defined(__LP64__) || defined(__x86_64__)
      // 64-bit build assumes pointers are always 64-bit
      #define LZHAM_PLATFORM_PC_X64 1
      #define LZHAM_64BIT_POINTERS 1
      #define LZHAM_CPU_HAS_64BIT_REGISTERS 1
   #else
      #define LZHAM_PLATFORM_PC_X86 1
      #define LZHAM_64BIT_POINTERS 0
      #define LZHAM_CPU_HAS_64BIT_REGISTERS 0
   #endif

   #define LZHAM_USE_UNALIGNED_INT_LOADS 1

   #if __BIG_ENDIAN__
      #define LZHAM_BIG_ENDIAN_CPU 1
   #else
      #define LZHAM_LITTLE_ENDIAN_CPU 1
   #endif

   #define LZHAM_USE_PTHREADS_API 1
   #define LZHAM_USE_GCC_ATOMIC_BUILTINS 1

   #define LZHAM_RESTRICT

   #if defined(__clang__)
      #define LZHAM_FORCE_INLINE inline
   #else
      #define LZHAM_FORCE_INLINE inline __attribute__((__always_inline__,__gnu_inline__))
   #endif

   #define LZHAM_NOTE_UNUSED(x) (void)x
#elif defined(ANDROID) && !defined(LZHAM_ANSI_CPLUSPLUS)
   // Generic GCC path for Android, GCC built-ins for atomic ops. Basically identical to iOS path.
   // Pthreads disabled because spin lock is missing..?
   #define LZHAM_PLATFORM_PC 0
   
   #if defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__)
      #define LZHAM_PLATFORM_PC_X64 0
      #define LZHAM_64BIT_POINTERS 1
      #define LZHAM_CPU_HAS_64BIT_REGISTERS 1
   #else
      #define LZHAM_PLATFORM_PC_X86 0
      #define LZHAM_64BIT_POINTERS 0
      #define LZHAM_CPU_HAS_64BIT_REGISTERS 0
   #endif
   
   #define LZHAM_USE_UNALIGNED_INT_LOADS 0
   
   #if __BIG_ENDIAN__
      #define LZHAM_BIG_ENDIAN_CPU 1
   #else
      #define LZHAM_LITTLE_ENDIAN_CPU 1
   #endif
   
   #define LZHAM_USE_PTHREADS_API 0
   #define LZHAM_USE_GCC_ATOMIC_BUILTINS 1
   
   #define LZHAM_RESTRICT
   
   #if defined(__clang__)
      #define LZHAM_FORCE_INLINE inline
   #else
      #define LZHAM_FORCE_INLINE inline __attribute__((__always_inline__,__gnu_inline__))
   #endif
   
   #define LZHAM_NOTE_UNUSED(x) (void)x
#else
#ifndef _MSC_VER
   #warning Building as vanilla ANSI-C/C++, multi-threaded compression is disabled! Please configure lzhamdecomp/lzham_core.h.
#endif

   // --- Vanilla ANSI-C/C++
   // No threading support, unaligned loads are NOT okay, no atomic ops.
   #if defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__)
      #define LZHAM_64BIT_POINTERS 1
      #define LZHAM_CPU_HAS_64BIT_REGISTERS 1
   #else
      #define LZHAM_64BIT_POINTERS 0
      #define LZHAM_CPU_HAS_64BIT_REGISTERS 0
   #endif

   #define LZHAM_USE_UNALIGNED_INT_LOADS 0

   #if __BIG_ENDIAN__
      #define LZHAM_BIG_ENDIAN_CPU 1
   #else
      #define LZHAM_LITTLE_ENDIAN_CPU 1
   #endif

   #define LZHAM_USE_GCC_ATOMIC_BUILTINS 0
   #define LZHAM_USE_WIN32_ATOMIC_FUNCTIONS 0

   #define LZHAM_RESTRICT
   #define LZHAM_FORCE_INLINE inline

   #define LZHAM_NOTE_UNUSED(x) (void)x
#endif

#if LZHAM_LITTLE_ENDIAN_CPU
   const bool c_lzham_little_endian_platform = true;
#else
   const bool c_lzham_little_endian_platform = false;
#endif

const bool c_lzham_big_endian_platform = !c_lzham_little_endian_platform;

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#if !defined(__APPLE__) && !defined(__FreeBSD__)
#include <malloc.h>
#endif
#include <stdarg.h>
#include <memory.h>
#include <limits.h>
#include <algorithm>
#include <errno.h>

#include "lzham.h"
#include "lzham_config.h"
#include "lzham_types.h"
#include "lzham_assert.h"
#include "lzham_platform.h"

#include "lzham_helpers.h"
#include "lzham_traits.h"
#include "lzham_mem.h"
#include "lzham_math.h"
#include "lzham_utils.h"
#include "lzham_vector.h"
