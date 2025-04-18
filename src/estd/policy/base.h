#pragma once

#include "../internal/raw/cstddef.h"

#include "rfc.h"

namespace estd { namespace internal {

// For allocators/containers specifically
// DEBT: Wants a better name and better namespace
// EXPERIMENTAL - some inner discord as to how allocators and policies get along.  Idea though
// is that policy is a configuration point of sorts which then allocators will specialize upon
struct policy_base
{
    static constexpr const bool is_singular = false;
    static constexpr const bool is_resizable = false;
    static constexpr const bool is_const = false;

    using size_type = size_t;

    // - emulated/shim (MAY perform lock)
    // - unavailable (MUST NOT perform lock)
    // - required (MUST perform lock)
    static constexpr rfc::terms locking = rfc::terms::must_not;
};

struct layer1_policy_base : policy_base { };

struct layer3_policy_base : policy_base
{
    static constexpr const bool is_resizable = true;
};

}}
