#pragma once

#include "../../../flags.h"

namespace estd { namespace detail { namespace impl {

// NOTE: Can't activate fn_options unless we also revise v2::function signature,
// which means virtual, fnptr1 and fnptr2 must all match
enum fn_options
{
    FN_NONE2,
    FN_COPY,
    FN_MOVE,
    FN_DTOR,

    FN_DEFAULT = FN_NONE2,
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

#ifndef FEATURE_ESTD_GH135
#define FEATURE_ESTD_GH135 1
#endif

}}}