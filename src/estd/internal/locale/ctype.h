/**
 *
 * References:
 *
 * 1. cplusplus.com/reference/locale/ctype
 * 2. https://en.cppreference.com/w/cpp/locale/ctype_char/is.html
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
    using mask = uint8_t;

    static constexpr mask space = 0x01;
    static constexpr mask digit = 0x02;
    static constexpr mask alpha = 0x04;
    static constexpr mask punct = 0x08;
    static constexpr mask upper = 0x10;
    static constexpr mask lower = 0x20;
    static constexpr mask xdigit = 0x40;

    // Not yet used - see https://en.cppreference.com/w/cpp/string/byte/isblank.html
    static constexpr mask blank = 0x80;

    static constexpr mask alnum = alpha | digit;
    static constexpr mask graph = alnum | punct;
};

namespace internal {

template <class Char>
class ascii_ctype_base : public ctype_base
{
public:
    using char_type = Char;

    // These discrete helpers are not part of std
    static constexpr bool isspace(char_type ch) { return internal::ascii_isspace(ch); }
    static constexpr bool isupper(char_type ch) { return internal::ascii_isupper(ch); }
    static constexpr bool islower(char_type ch) { return internal::ascii_islower(ch); }
};

// specialization, deviating from standard in that locale is compile-time
// instead of runtime
// This has a number of implications, but mainly we are hard-wired
// to default-ASCII behaviors.  Ultimately this will be an issue but
// we can build out ctype at that time
// strongly implies a layer1 behavior
template <class Locale>
class ctype<char, Locale,
    enable_if_t<internal::is_compatible_with_classic_locale<Locale>::value>> :
    public ascii_ctype_base<char>
{
    using base_type = ascii_ctype_base<char>;
    using locale_type = Locale;

public:
    //static locale::id id;

    static constexpr char widen(char c) { return c; }

    // "returns whether c belongs to any of the categories specified in bitmask m" [1]
    // 06JUL25 MB DEBT: Serves me right for using cplusplus.com, probably it's a bit more mutually
    // exclusive (not sure) as per [2]
    static ESTD_CPP_CONSTEXPR(14) bool is(mask m, char ch)
    {
        if(m & space)
        {
            if(base_type::isspace(ch)) return true;
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
            if(base_type::isupper(ch)) return true;
        }
        else if(m & lower)
        {
            if(base_type::islower(ch)) return true;
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
