#pragma once

#include "fwd.h"
#include "cbase.h"
#include "numpunct.h"

#include "../charconv.hpp"
#include "iterated/num_put.h"

namespace estd {

// In development, not ready, so marked as internal
namespace internal {

//constexpr
inline unsigned get_base2(const ios_base::fmtflags basefield)
{
    // DEBT: Consider a switch statement because we can, but might be slightly fragile
    if(basefield == ios_base::hex)
    {
        return 16;
    }
    else if(basefield == ios_base::dec)
    {
        return 10;
    }
    else
    {
        return 8;
        // oct
        // basefield mask is guaranteed to produce one of these 3 options
        // DEBT: Ensure there's a unit test to that effect elsewhere
    }
}

constexpr unsigned get_base(const ios_base::fmtflags basefield)
{
    return basefield == ios_base::hex ? 16 :
        basefield == ios_base::dec ? 10 :
        8;
}

// Integer core of num_put, no fill/width padding
template <class Locale = classic_locale_type,
    cbase_policies policy = cbase_policies::CBASE_POLICY_DEFAULT,
    class Enabled = void>
class integer_put;


template <class Locale, cbase_policies policy>
class integer_put<Locale, policy, estd::enable_if_t<is_ascii_compatible(Locale::encoding)> >
{
public:
    /// Low level integer -> string call.  Be mindful, uses to_char_opt so
    /// result.ptr is non standard in its behavior
    /// @tparam OutputIt
    /// @tparam T
    /// @param first
    /// @param last
    /// @param str
    /// @param value
    /// @return
    template <class OutputIt, class T>
    static detail::to_chars_result<OutputIt> to_chars(OutputIt first, OutputIt last,
        const ios_base& str,
        const T& value)
    {
        using iter_type = OutputIt;
        using char_type = typename iterator_traits<iter_type>::value_type;
        const unsigned base = get_base(str.flags() & ios_base::basefield);

        if(base <= 10 &&
            cbase_policies(policy & cbase_policies::CBASE_POLICY_HEX_ALWAYS) == 0)
        {
            return internal::to_chars_integer_opt(
                first, last, value, internal::base_provider<>(base),
                // NOTE: lowercase policy doesn't matter and is just a placeholder
                // for base10 and lower
                cbase_utf<char_type, 10, CBASE_POLICY_CASE_LOWER>{});
        }
        else
        {
            const bool uppercase = str.flags() & ios_base::uppercase;

            return internal::to_chars_integer_opt(
                first, last, value, internal::base_provider<>(base),
                cbase_utf<char_type, 36,
                    cbase_policies(policy & cbase_policies::CBASE_POLICY_CASE_MASK)>(
                    uppercase ? CBASE_POLICY_CASE_UPPER : CBASE_POLICY_CASE_LOWER));
        }
    }
};

// Fallback for non ASCII systems.  Untested, and doesn't support hex
template <class Locale, cbase_policies policy>
class integer_put<Locale, policy, estd::enable_if_t<!is_ascii_compatible(Locale::encoding)> >
{
public:
    template <class OutputIt, class T>
    static to_chars_result to_chars(OutputIt first, OutputIt last,
        const ios_base& str,
        const T& value)
    {
        using iter_type = OutputIt;
        using char_type = typename iterator_traits<iter_type>::value_type;
        const unsigned base = get_base(str.flags() & ios_base::basefield);

        return internal::to_chars_integer_opt(
            first, last, value, internal::base_provider<>(base),
            cbase<char_type, 10, Locale>{});
    }
};

}

template <class Char, class OutputIt,
    class Locale = internal::classic_locale_type,
    internal::cbase_policies = internal::CBASE_POLICY_DEFAULT>
class num_put;

template <class Char, class OutputIt, class Locale, internal::cbase_policies policy>
class num_put
{
public:
    typedef Char char_type;
    typedef OutputIt iter_type;

private:
    template <class T>
    static iter_type put_integer(iter_type out, const ios_base& str,
        char_type fill,
        const T& value)
    {
        // Hardcode to base 8 since that's the biggest version
        // +1 for potential - sign
#if FEATURE_ESTD_OSTREAM_OCTAL
        constexpr unsigned N = estd::numeric_limits<T>::template length<8>::value + 1;
#else
        constexpr unsigned N = estd::numeric_limits<T>::template length<10>::value + 1;
#endif

        // No extra space for null terminator, to_chars_opt gives us start/end ptrs
        char_type buffer[N];

        using helper = internal::integer_put<Locale, policy>;

        to_chars_result result = helper::to_chars(buffer, buffer + N, str, value);

        unsigned width = str.width();
        const unsigned result_width = buffer + N - result.ptr;

        if(width > result_width)
        {
            width -= result_width;

            const ios_base::fmtflags adj = str.flags() & ios_base::adjustfield;
            if(adj == ios_base::right)
            {
                out = fill_n(out, width, fill);
                return copy(result.ptr, buffer + N, out);
            }
            else if(adj == ios_base::left)
            {
                out = copy(result.ptr, buffer + N, out);
                return fill_n(out, width, fill);
            }
            else
            {
                // DEBT: 'internal' flavor, needs work
                return out;
            }
        }
        else
            return copy(result.ptr, buffer + N, out);
    }

    template <unsigned base, class T>
    static iter_type put_float(iter_type out, const ios_base& str, char_type fill, T value)
    {
#if __cpp_static_assert
        static_assert(!is_floating_point<T>::value, "floating point not yet supported");
#endif
        return out;
    }

public:
    static iter_type put(iter_type out, const ios_base& str, char_type fill,
        bool value)
    {
        if(str.flags() & ios_base::boolalpha)
        {
            using np = numpunct<char_type, Locale>;
            // DEBT: We may prefer plain old const char* instead
            layer2::const_string _v = value ? np::truename() : np::falsename();
            const char* v = _v.data();
            while(*v != 0) *out++ = *v++;
        }
        else
        {
            using cb = cbase<char_type, 2, Locale>;
            *out++ = cb::to_char(value);
        }

        return out;
    }

    template <class T>
    // type filter not actually needed at the moment, but will probably come in handy when
    // floats get involved
    static constexpr typename enable_if<numeric_limits<T>::is_integer, iter_type>::type
    //static iter_type
    put(iter_type out, const ios_base& str, char_type fill, T value)
    {
        return put_integer(out, str, fill, value);
    }

    template <class T>
    // type filter not actually needed at the moment, but will probably come in handy when
    // floats get involved
    static typename enable_if<is_floating_point<T>::value, iter_type>::type
    //static iter_type
    put(iter_type out, const ios_base& str, char_type fill, T value)
    {
        return put_float(out, str, fill, value);
    }
};


}
