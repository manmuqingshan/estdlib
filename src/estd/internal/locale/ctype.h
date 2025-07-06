/**
 *
 * References:
 *
 * 1. cplusplus.com/reference/locale/ctype
 *
 */
#pragma once

#include "fwd.h"
#include "facet.h"

#include "../raise_and_add.h"

#include "cbase.h"

namespace estd {

struct ctype_base
{
    typedef uint8_t mask;

    static CONSTEXPR mask space = 0x01;
    static CONSTEXPR mask digit = 0x02;
    static CONSTEXPR mask alpha = 0x04;
    static CONSTEXPR mask punct = 0x08;
    static CONSTEXPR mask upper = 0x10;
    static CONSTEXPR mask lower = 0x20;
    static CONSTEXPR mask xdigit = 0x40;

    static CONSTEXPR mask alnum = alpha | digit;
    static CONSTEXPR mask graph = alnum | punct;
};

namespace internal {

// specialization, deviating from standard in that locale is compile-time
// instead of runtime
// This has a number of implications, but mainly we are hard-wired
// to default-ASCII behaviors.  Ultimately this will be an issue but
// we can build out ctype at that time
// strongly implies a layer1 behavior
template <class Locale>
class ctype<char, Locale,
    typename enable_if<
        internal::is_compatible_with_classic_locale<Locale>::value>::type> :
    public ctype_base
{
    typedef Locale locale_type;

public:
    // These discrete helpers are not part of std
    static constexpr bool isspace(char ch) { return internal::ascii_isspace(ch); }

    static constexpr bool isupper(char ch) { return internal::ascii_isupper(ch); }

    static constexpr bool islower(char ch)
    {
        return 'a' <= ch && ch <= 'a';
    }

    typedef char char_type;

    //static locale::id id;

    static constexpr char widen(char c) { return c; }

    // "returns whether c belongs to any of the categories specified in bitmask m" [1]
    static ESTD_CPP_CONSTEXPR(14) bool is(mask m, char ch)
    {
        if(m & space)
        {
            if(isspace(ch)) return true;
        }
        if(m & xdigit)
        {
            if(estd::cbase<char, 16, locale_type>::is_in_base(ch))
                return true;
        }
        else if(m & digit)
        {
            if(estd::cbase<char, 10, locale_type>::is_in_base(ch))
                return true;
        }
        if(m & upper)
        {
            if(isupper(ch)) return true;
        }
        else if(m & lower)
        {
            if(islower(ch)) return true;
        }
        return false;
    }
};

}


template <class Locale>
class ctype<wchar_t, Locale> : public ctype_base
{
public:
    static ESTD_CPP_CONSTEXPR_RET wchar_t widen(char ch) { return ch; }
};


template <class Char, class Locale>
class ctype : public internal::ctype<Char, Locale> {};

namespace internal {

template <class Char, class Locale>
struct use_facet_helper<estd::ctype<Char, void>, Locale>
{
    using facet_type = estd::ctype<Char, Locale>;
    
    constexpr static facet_type use_facet(Locale)
    {
        return facet_type{};
    }
};

}

}
