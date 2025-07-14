#pragma once

//#include "../cctype.h"
#include "../locale/ctype.h"
#include "../raise_and_add.h"

#include "result.h"

namespace estd { namespace internal {

template<class Cbase, bool sto_mode = false, class T, class CharIt>
detail::from_chars_result<CharIt> from_chars_integer(CharIt first, CharIt last,
    T& value,
    const unsigned short base = Cbase::base())
{
    using cbase_type = Cbase;
    using optional_type =  typename cbase_type::optional_type;
    using result_type = detail::from_chars_result<CharIt>;

    // DEBT: Expand this to allow any numeric type, we'll have to make specialized
    // versions of raise_and_add to account for that
    static_assert(estd::is_integral<T>::value, "T must be integral type");

    CharIt current = first;
    bool negate;
    // "If no characters match the pattern or if the value obtained by parsing
    // the matched characters is not representable in the type of value, value is unmodified," [1]
    T local_value = 0;

    // DEBT: Spec calls for octal leading '0' parsing when base = 0.  A fallback to decimal
    // is also expected.  Specifically:
    // "pattern identical to the one used by std::strtol" [1]
    // "If the value of base is 0, the numeric base is auto-detected: if the prefix is 0, [...]
    //  otherwise the base is decimal" [2]

    // "leading whitespace is not ignored" [1]
    // Only eat whitespace in sto mode
    while (sto_mode && estd::internal::ascii_isspace(*current))
        ++current;

    // NOTE: A little odd, https://en.cppreference.com/w/cpp/string/byte/strtoul.html
    // indicates '-' is OK, but how could that be?
    if (estd::is_signed<T>::value)
    {
        negate = *current == '-';

        if (negate) ++current;
    }

    if (sto_mode && *current == '+')    ++current;

    while (current != last)
    {
        const optional_type digit = cbase_type::from_char(*current, base);
        if (digit.has_value())
        {
            bool success = raise_and_add(local_value, base, digit.value());

            // If we didn't succeed, that means we overflowed
            if (!success)
            {
                // skip to either end or next spot which isn't a number
                // "ptr points at the first character not matching the pattern." [1]
                while (++current != last && cbase_type::is_in_base(*current)) {}

                return result_type{current, estd::errc::result_out_of_range};
            }
        }
        // If first character isn't a digit, error out immediately
        else if(current == first)
        {
            return result_type{current, estd::errc::invalid_argument};
        }
        else
        {
            if(sto_mode)
            {
                // TODO: Look for 0x, 0X
            }

            value = local_value;
            return result_type{current, estd::errc(0)};
        }
        ++current;
    }

    // prepend with constexpr so we can optimize out non-signed flavors
    if (estd::is_signed<T>::value && negate)
        local_value = -local_value;

    value = local_value;
    return result_type{last,estd::errc(0)};
}

}}