#pragma once

#include "fwd/chrono.h"
#include "chrono/features.h"
#include "chrono/duration.h"
#include "chrono/time_point.h"
#include "common_type.h"

namespace estd { namespace chrono {

namespace internal {

template <class Clock>
struct clock_traits
{
    template <class T>
    constexpr static T adjust_epoch(T t) { return t; }
};

}


}}
