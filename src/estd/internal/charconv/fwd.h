#pragma once

#include "../type_traits.h"

namespace estd {

namespace detail {

template <typename CharIt>
struct from_chars_result;

}

using from_chars_result = detail::from_chars_result<const char*>;

template <class Int, bool sto_mode = false>
typename estd::enable_if<estd::numeric_limits<Int>::is_integer, from_chars_result>::type
    from_chars(const char* first,
        const char* last,
        Int& value,
        const int base = 10);

}