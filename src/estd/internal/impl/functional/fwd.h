#pragma once

#include "../../../flags.h"

namespace estd { namespace detail { namespace impl {

enum fn_options
{
    FN_NONE,

    FN_COPY,            // fn(const fn&)
    FN_MOVE,            // fn(fn&&)
    FN_DTOR,            // ~fn
    FN_AUTO_DTOR,       // ~fn *always* after execution (opt mode)

    // TODO: Consider fnptr1, fnptr2 and virtual specifiers here too

    FN_DEFAULT = FN_NONE,
};

ESTD_FLAGS(fn_options)

template <typename F, fn_options = FN_DEFAULT>
struct function_fnptr1;

template <typename F, fn_options = FN_DEFAULT>
struct function_fnptr1_opt;

template <typename F, fn_options = FN_DEFAULT>
struct function_fnptr2;

template <typename F, fn_options = FN_DEFAULT>
struct function_fnptr2_opt;

template <typename F, fn_options = FN_DEFAULT>
struct function_virtual;

// 21JUL25 MB In progress, coming along:
// 1. fnptr1 no #135 implementation
// 2. fnptr2 #135 implementation, needs testing and refinement
// 3. virtual #135 implementation, needs testing
#ifndef FEATURE_ESTD_GH135
#define FEATURE_ESTD_GH135 1
#endif

}}}