// Non-standard facet for character-to-numeric base conversion
#pragma once

//#include <estd/string.h>
#include <estd/optional.h>

#include "fwd.h"
#include "facet.h"
#include "utility.h"
#include "../deduce_fixed_size.h"
#include "../cctype.h"

#if __AVR__
#include "../../exp/pgm/array.h"
#endif

// EXPERIMENTAL
#define FEATURE_ESTD_CBASE_ARRAY 0

// TODO: A future task.  Japanese character set has 3 different (at least) sets of numerals:
// 1.  Regular ASCII style
// 2.  Kanji style
// 3.  An Arabic (English) version which appears to be in with the other Kanji characters

namespace estd {

//template <typename TChar, unsigned b, class TLocale>
//struct cbase;

namespace internal {

template <typename Char, unsigned b, cbase_policies>
struct cbase_utf_base;

template <typename Char>
struct cbase_set {};

// Hex converter assist
// NOTE: Only works for ASCII compatible scenarios.  Not DEBT because:
// a) what useful scenarios use hex that aren't ASCII compatible?  EBCDIC etc. don't seem to make the grade
// b) feels like FEATURE_ESTD_CBASE_ARRAY will displace the character math flavor for hex conversions anyway
template <cbase_policies c>
struct cbase_casing_base
{
    static constexpr cbase_casing casing() { return c; }

    // DEBT: Something like a constexpr if to avoid ctor call altogether
    // would be better
    constexpr cbase_casing_base(cbase_casing = {}) {}

    // Upper or lower case A, depending on configuration
    constexpr char a_char() const
    {
        return c == CBASE_POLICY_CASE_UPPER ? 'A' : 'a';
    }

};

template <>
struct cbase_casing_base<CBASE_POLICY_CASE_DYNAMIC>
{
    char a_char_;

    constexpr cbase_policies casing() const
    {
        return a_char_ == 'A' ? CBASE_POLICY_CASE_UPPER : CBASE_POLICY_CASE_LOWER;
    }

    explicit constexpr cbase_casing_base(cbase_casing c = CBASE_POLICY_CASE_LOWER) :
        a_char_{c == CBASE_POLICY_CASE_UPPER ? 'A' : 'a'}
    {}

    constexpr char a_char() const
    {
        return a_char_;
    }
};


// EXPERIMENTAL
template <>
struct cbase_set<char>
{
#if __AVR__
    static constexpr estd::v0::pgm_array<char, 16> lset PROGMEM
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f'
    };
#else
    static constexpr char lset[] = "0123456789abcdef";
    static constexpr char uset[] = "0123456789ABCDEF";
#endif
};


template <typename Char, unsigned b, cbase_policies policy>
struct cbase_utf_base : cbase_casing_base<policy>,
    cbase_set<Char>
{
    using base_type = cbase_casing_base<policy>;
    using char_type = Char;

    typedef int16_t int_type;

    //typedef int_type optional_type;
    //inline static CONSTEXPR int_type eol() { return -1; }

    // DEBT: Unsure why MSVC hates our layer1 flavor, but for now regular
    // optional compiles.
    // DEBT: Something a little weird going on with eol reaching into optional_type.
    // Document it to undo debt
#if defined(_MSC_VER)
    typedef const estd::optional<int_type> optional_type;
#else
    typedef const estd::layer1::optional<int_type, -1> optional_type;
#endif

    inline static CONSTEXPR int_type eol() { return optional_type::null_value; }

    static inline CONSTEXPR unsigned base() { return b; }

    ESTD_CPP_FORWARDING_CTOR(cbase_utf_base)
};

template <typename Char, cbase_policies policy, unsigned b>
struct cbase_utf<Char, b, policy, estd::internal::Range<b <= 10> > :
    cbase_utf_base<Char, b, CBASE_POLICY_CASE_LOWER>    // hard code to lower since doesn't matter without hex
{
    typedef cbase_utf_base<Char, b, CBASE_POLICY_CASE_LOWER> base_type;
    typedef typename base_type::int_type int_type;
    typedef Char char_type;

    ESTD_CPP_FORWARDING_CTOR(cbase_utf)


    // adapted from GNUC
    static ESTD_CPP_CONSTEXPR_RET bool is_in_base(char_type c, const unsigned _base = b)
    {
        return '0' <= c && c <= ('0' + (char_type)(_base - 1));
    }

    /// No bounds checking performed on conversion.
    /// \param c
    /// \return Character value of 0-9 converted from char to int.  Any other character value
    /// results in an int_type either < 0 or > 10
    static ESTD_CPP_CONSTEXPR_RET int_type from_char_raw(char_type c)
    {
        return c - '0';
    }

    static ESTD_CPP_CONSTEXPR_RET char_type to_char(int_type v)
    {
        return '0' + v;
    }

    static ESTD_CPP_CONSTEXPR_RET typename base_type::optional_type
    from_char(char_type c, const int _base = b)
    {
        return is_in_base(c, _base) ?
               from_char_raw(c) :
               //typename base_type::optional_type::null_value();
               base_type::eol();
    }
};


// DEBT: In fact, really we have only two cbase_utfs - a base 10 one and a base 36 one.
// On one level, this range is almost overly fancy and showing off when we could merely
// have specialized on b directly.
// On the other hand, there is a convenience here in that consumers knowing at compile time
// what base they are using can auto-feed 'base' to things like 'from_chars_integer'.
template <typename Char, cbase_policies policy, unsigned b>
struct cbase_utf<Char, b, policy, estd::internal::Range<(b > 10 && b <= 36)> > :
    cbase_utf_base<Char, b, policy>
{
    typedef cbase_utf_base<Char, b, policy> base_type;
    typedef typename base_type::int_type int_type;
    typedef Char char_type;

    ESTD_CPP_FORWARDING_CTOR(cbase_utf)

    static ESTD_CPP_CONSTEXPR_RET bool isupper(char_type c, const unsigned _base = b)
    {
        return 'A' <= c && c <= ('A' + char_type(_base - 11));
    }

    // upper or lower - untested
    static constexpr bool isalpha(char_type c, const unsigned _base)
    {
        return isupper(c & ~0x20, _base);
    }

    static ESTD_CPP_CONSTEXPR_RET bool islower(char_type c, const unsigned _base = b)
    {
        return 'a' <= c && c <= ('a' + char_type(_base - 11));
    }

    static inline CONSTEXPR bool is_in_base(char_type c, const unsigned _base = b)
    {
        // DEBT: We really want to consider ctype's isdigit here
        return estd::internal::ascii_isdigit(c) ||
               isupper(c, _base) ||
               islower(c, _base);
    }

    static inline typename base_type::optional_type
    from_char(estd::remove_const_t<char_type> c, const unsigned short _base = b)
    {
        // DEBT: We really want to consider ctype's isdigit, toupper and islower here
        if (estd::internal::ascii_isdigit(c)) return c - '0';

        c &= ~0x20;     // Turns lowercase to upper

        if (isupper(c, _base)) return c - 'A' + 10;

        //if (islower(c, _base)) return c - 'a' + 10;

        return estd::nullopt;
    }

    static inline int_type from_char_raw(char_type c)
    {
        if (c <= '9')
            return c - '0';
        else if (c <= 'Z')
            return c - 'A' + 10;
        else
            return c - 'a' + 10;
    }

    ESTD_CPP_CONSTEXPR_RET char_type to_char(int_type v) const
    {
#if FEATURE_ESTD_CBASE_ARRAY
        return base_type::lset[v];
#else
        return v < 10 ?
            ('0' + v) :
            char_type(base_type::a_char()) + (v - 10);
#endif
    }
};


template <typename Char, unsigned b,
    internal::locale_code::values lc,
    internal::encodings::values encoding>
struct cbase<Char, b, internal::locale<lc, encoding>,
    typename estd::enable_if<
        encoding == internal::encodings::ASCII ||
        encoding == internal::encodings::UTF8 ||
        encoding == internal::encodings::UTF16>::type> :
    cbase_utf<Char, b, CBASE_POLICY_CASE_LOWER>
{
    // EXPERIMENTAL
    using locale_type = internal::locale<lc, encoding>;
};

template <class TChar, unsigned b, class TLocale>
struct use_facet_helper<estd::cbase<TChar, b, void>, TLocale>
{
    typedef estd::cbase<TChar, b, TLocale> facet_type;

    inline static facet_type use_facet(TLocale)
    {
        return facet_type();
    }
};

}

}
