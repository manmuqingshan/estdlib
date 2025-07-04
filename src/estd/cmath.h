#pragma once

#include "internal/platform.h"

#if FEATURE_STD_CMATH
#include <cmath>
#endif

namespace estd {

// DEBT: Extremely crude raise-power function.  Improve by way of
// https://stackoverflow.com/questions/18581560/any-way-faster-than-pow-to-compute-an-integer-power-of-10-in-c

template <class T>
inline ESTD_CPP_CONSTEXPR(14) T pow(T base, unsigned exp)
{
    T v = base;
    while(--exp)    v *= base;
    return v;
}

}
