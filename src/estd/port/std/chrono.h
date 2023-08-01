#pragma once

#include "../../internal/chrono.h"

#ifdef FEATURE_ESTD_CHRONO

namespace estd { namespace chrono {

#ifdef FEATURE_STD_CHRONO
template <class ToDuration, class Rep, class Period>
inline ToDuration duration_cast(const std::chrono::duration<Rep, Period>& d)
{
    typedef ratio<Period::num, Period::den> period_type;

    duration<Rep, period_type> our_d(d);

    return duration_cast<ToDuration>(our_d);
}
#endif

}}

#endif