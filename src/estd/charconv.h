/*
 * @file
 */
#pragma once

#include "system_error.h"
#include "cctype.h"
#include "type_traits.h"
#include "internal/charconv.hpp"
#include "algorithm.h"

namespace estd {

namespace internal {

// We take both compile time and runtime base because compile-time base really just splits into
// two - base10 and base36.  Reason is that cbase itself still takes a runtime parameter - so
// the former deals with numeric, while the latter deals with alphanumeric

template<unsigned b, class T, class CharIt>
inline estd::detail::from_chars_result<CharIt> from_chars_integer(CharIt first, CharIt last,
    T& value,
    const unsigned short base = b)
{
    return internal::from_chars_integer<
        cbase<char, b, internal::classic_locale_type> >(first, last, value, base);
}

template <unsigned b, class Int, class CharIt>
inline typename estd::enable_if<estd::numeric_limits<Int>::is_integer, detail::to_chars_result<CharIt> >::type
to_chars(CharIt first, CharIt last, Int value, unsigned base)
{
    typedef typename iterator_traits<CharIt>::value_type char_type;
    typedef cbase<char_type, b, internal::classic_locale_type> cbase_type;
    return internal::to_chars_integer_opt<cbase_type>(first, last, value, internal::base_provider<>(base));
}


}

namespace detail {

template <unsigned b, class Int, class CharIt>
inline typename estd::enable_if<estd::numeric_limits<Int>::is_integer, to_chars_result<CharIt> >::type
to_chars(CharIt first, CharIt last, Int value)
{
    typedef typename iterator_traits<CharIt>::value_type char_type;
    typedef cbase<char_type, b, internal::classic_locale_type> cbase_type;
    return internal::to_chars_integer_opt<cbase_type>(first, last, value, internal::base_provider<b>());
}

}


template <class TInt>
inline typename estd::enable_if<estd::numeric_limits<TInt>::is_integer, from_chars_result>::type
    from_chars(const char* first,
        const char* last,
        TInt& value,
        const int base = 10)
{
    if(base > 10)
        return internal::from_chars_integer<36>(first, last, value, base);
    else
        return internal::from_chars_integer<10>(first, last, value, base);
}

// TODO: Needs bounds check on to_chars

/// Deviates from regular to_chars in that 'ptr' refers to start rather than one past end,
/// and 'last' is the static non-deviating end
/// @tparam TInt
/// @param first
/// @param last
/// @param value
/// @param base
/// @return
template <class Int>
typename enable_if<numeric_limits<Int>::is_integer, to_chars_result>::type
    to_chars_opt(char* first, char* last, Int value, const int base = 10)
{
    if(base > 10)
        return internal::to_chars<36>(first, last, value, base);
    else
        return internal::to_chars<10>(first, last, value, base);
}

template <class TInt>
inline typename estd::enable_if<estd::numeric_limits<TInt>::is_integer, to_chars_result>::type
    to_chars(char* first, char* last, TInt value, const int base = 10)
{
    // uses buffer "in place" and moves it over, since to_chars_opt
    // loves to go right-to-left

    to_chars_result opt_result = to_chars_opt(first, last, value, base);

    // TODO: Double check that we really want 'last + 1' and whether
    // we do or not, document why
    opt_result.ptr = estd::copy(opt_result.ptr, last + 1, first);

    return opt_result;
}

template <class TInt>
to_chars_result to_chars_exp(char* first, char* last, TInt value, const int base = 10)
{
    if(base > 10)
        return internal::to_chars_integer<36>(first, last, value, base);
    else
        return internal::to_chars_integer<10>(first, last, value, base);
}



}
