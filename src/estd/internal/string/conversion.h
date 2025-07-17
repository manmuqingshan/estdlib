#pragma once

//#include "../../charconv.h"
#include "../fwd/string.h"
#include "../charconv/fwd.h"
#include "../charconv/result.h"

#ifndef FEATURE_ESTD_GH134
#define FEATURE_ESTD_GH134 1
#endif

namespace estd { namespace internal {

template <class Int, ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
Int stoi(
    const detail::basic_string<Impl>& str,
    size_t* pos = nullptr, int base = 10)
{
    Int v;
    using char_type = typename Impl::policy_type::char_traits::char_type;

    const char_type* data = str.data();
    const from_chars_result r = estd::from_chars<Int, true>(data, data + str.size(), v, base);

    // Backchannel error status as '0' chars processed since we don't do exceptions
    if(pos)     *pos = r.ec == 0 ? (r.ptr - data) : 0;

    return v;
}

}

#if FEATURE_ESTD_GH134
template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
ESTD_CPP_CONSTEXPR(14) long stol(
    const detail::basic_string<Impl>& str,
    size_t* pos = nullptr, int base = 10)
{
    return internal::stoi<long>(str, pos, base);
}

template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
ESTD_CPP_CONSTEXPR(14) int stoi(
    const detail::basic_string<Impl>& str,
    size_t* pos = nullptr, int base = 10)
{
    return internal::stoi<int>(str, pos, base);
}

template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
ESTD_CPP_CONSTEXPR(14) unsigned long stoul(
    const detail::basic_string<Impl>& str,
    size_t* pos = nullptr, int base = 10)
{
    return internal::stoi<unsigned long>(str, pos, base);
}
#endif

}