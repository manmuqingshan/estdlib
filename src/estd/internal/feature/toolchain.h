#pragma once

#include "../../port/toolchain.h"

// feature area different from toolchain/gnuc.h - the latter focuses more
// on compiler compatibility

#if defined(__GNUC_PREREQ)

// AVR compiler appears to have incomplete implementation of __has_builtin
// DEBT: According to https://stackoverflow.com/questions/3944505/detecting-signed-overflow-in-c-c
// contents of the result (&__val in this case) are undefined on overflow.  That might be an
// issue down the line if we're depending on what we've gathered so far.
// DEBT: For some unknown reason, GCC < 10 doesn't report it has builtins, even though it does.
// THe following link indicates at least 6.2 has it.  Unknown how far back it goes, though I've
// seen discussions going back to 5.0
// https://gcc.gnu.org/onlinedocs/gcc-6.2.0/gcc/Integer-Overflow-Builtins.html
#if __GNUC_PREREQ(6, 2) || defined(__has_builtin) && !defined(__AVR__)
#ifndef FEATURE_TOOLCHAIN_OVERFLOW_BUILTIN
#define FEATURE_TOOLCHAIN_OVERFLOW_BUILTIN 1
#endif
#endif

#if __AVR__
#define FEATURE_TOOLCHAIN_BUILTIN_BSWAP16  1
#define FEATURE_TOOLCHAIN_BUILTIN_BSWAP32  1
#define FEATURE_TOOLCHAIN_BUILTIN_BSWAP64  1
#else
#define FEATURE_TOOLCHAIN_BUILTIN_BSWAP16  __has_builtin(__builtin_bswap16) 
#define FEATURE_TOOLCHAIN_BUILTIN_BSWAP32  __has_builtin(__builtin_bswap32) 
#define FEATURE_TOOLCHAIN_BUILTIN_BSWAP64  __has_builtin(__builtin_bswap64) 
#endif

#endif