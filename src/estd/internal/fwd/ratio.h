#pragma once

#include "../../cstdint.h"

namespace estd {

template<
    std::intmax_t Num,
    std::intmax_t Denom = 1
> class ratio;

namespace detail {

template <class R1, class R2>
struct ratio_add;

template <class R1, class R2>
struct ratio_divide;

template <class R1, class R2>
struct ratio_multiply;

}

#ifdef __cpp_alias_templates
template <class R1, class R2>
using ratio_multiply = typename detail::ratio_multiply<R1, R2>::type;

template <class R1, class R2>
using ratio_divide = typename detail::ratio_divide<R1, R2>::type;

template <class R1, class R2>
using ratio_add = typename detail::ratio_add<R1, R2>::type;
#endif

}