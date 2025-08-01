/***
 * State machine flavor of num_get
 *
 * References:
 *
 * 1. https://en.cppreference.com/w/cpp/locale/num_get/get
 *
 * Notes:
 *
 * "If Stage 2 was terminated by the test in==end, err|=std::ios_base::eofbit
 *  is executed to set the eof bit."
 *
 * This is somewhat nuanced.  This means that certain types such as integer, we plunge
 * forward until delimiter or EOF.  bool, however, we know where the end is - so EOF
 * happens less often with a bool.
 */
#pragma once

#include "../cbase.h"
#include "../numpunct.h"
#include "../fwd.h"
#include "../../ios_base.h"
#include "../../chooser.h"
#include "../../feature/num_get.h"
#include "../../../cmath.h"

#include "../../macro/push.h"

namespace estd { namespace iterated {

// DEBT: Consider a default Locale.
template <unsigned base, typename Char, class Locale>
struct num_get
{
    using locale_type = Locale;
    using char_type = Char;
    using cbase_type = cbase<char_type, base, locale_type>;
    using ctype_type = ctype<estd::remove_const_t<char_type>, locale_type>;

    typedef typename cbase_type::optional_type optional_type;
    typedef typename cbase_type::int_type int_type;
    typedef numpunct<estd::remove_const_t<char_type>, locale_type> numpunct_type;

    enum state
    {
        Start = 0,
        Header,
        FirstNegative,
        FirstPositive,
        NominalNegative,
        NominalPositive,
        Overflow,
        Complete
    };

    struct _state
    {
        // DEBT: Consider optimizing out somehow when not an integer
        unsigned decimal_place_ : 8;

        state state_ : 4;
        //bool is_signed : 1;

        _state() : decimal_place_(0), state_(Start) //, is_signed(false)
        {}

    } state_;

    // 'true_type' means this is an integer
    // 'false_type' means this is the unsigned flavor
    template <bool positive, typename T>
    inline static bool raise_and_add(int_type n, T& v, true_type, false_type)
    {
        // Undefined/bad state.  Same as 'default' case switch
        // DEBT: Logging this and other internal failures would be nice.  Can't do a static assert
        // since technically state machine might reach here
        if(!positive) return true;

        return estd::internal::raise_and_add(v, base, n);
    }

    // #1 'true_type' means this is an integer
    // #2 'true_type' means this is the signed flavor
    template <bool positive, typename T>
    ESTD_CPP_CONSTEXPR_RET static bool raise_and_add(int_type n, T& v, true_type, true_type)
    {
        return positive ?
           estd::internal::raise_and_add(v, base, n) :
           estd::internal::raise_and_sub(v, base, n);
    }

    // 'false_type' means this is a float or double
    // 'true_type' means this is the signed flavor
    template <bool positive, typename T>
    bool raise_and_add(int_type n, T& v, false_type, true_type)
    {
#if __cpp_static_assert
        static_assert(base == 10, "Only base 10 supported for floating point");
#endif

        v *= base;
        if(positive)
            v += n;
        else
            v -= n;

        if(state_.decimal_place_ != 0)
        {
            // DEBT: Side effects
            ++state_.decimal_place_;
        }

        return true;
    }

    /// Parse one character and apply it to 'v'
    /// @tparam positive
    /// @tparam T
    /// @param c
    /// @param err
    /// @param v
    /// @return false if 'c' is generally parseable as part of a number, true otherwise
    template <bool positive, typename T>
    bool nominal(char_type c, ios_base::iostate& err, T& v)
    {
        typedef estd::bool_constant<numeric_limits<T>::is_integer> is_integer;
        optional_type n = cbase_type::from_char(c);

        if(n.has_value())
        {
            if (!raise_and_add<positive>(n.value(), v, is_integer(), is_signed<T>()))
            {
                state_.state_ = Overflow;
                err |= ios_base::failbit;

                // DEBT: Consider consolidating this with code elsewhere
                // which assigns v = 0, though only if it improves code
                // efficiency
#if FEATURE_ESTD_NUM_GET_LWG23
                v = positive ?
                    estd::numeric_limits<T>::max() :
                    estd::numeric_limits<T>::min();
#endif
            }

            return false;
        }
        else
        {
            // TODO: Account for thousands separators
            // NOTE: Probably should use is_floating_point here instead, though so far
            // is_integer is in no danger of causing us grief
            if(is_integer() == false && c == numpunct_type::decimal_point())
            {
                state_.decimal_place_ = 1;
                return false;
            }
        }

        return true;
    }

    // integer variety (noop)
    template <typename T>
    static constexpr bool finalize(T&, true_type)
    {
        return {};
    }

    // floating point variety
    template <typename T>
    void finalize(T& v, false_type) const
    {
        if(state_.decimal_place_ != 0)
        {
#if FEATURE_STD_CMATH_NOTREADY
            // Works, just somehow slightly less precise.  Disabled until that is resolved
            const T mult = std::pow(base, -(state_.decimal_place_ - 1));
            v *= mult;
#else
            const T divider = estd::pow(base, state_.decimal_place_ - 1);

            v /= divider;
#endif
        }
    }

    template <class T>
    constexpr bool finalize(T& v) const
    {
        return finalize(v, bool_constant<numeric_limits<T>::is_integer>{}), false;
    }

    // NOTE: This method never sets eof bit
    // NOTE: Doing autoinit because compilers sometimes don't trust that we really do initialize
    // v, causing "maybe-uninitialized" warning.  To handle this, external parties may elect to
    // init to zero instead of us.
    // DEBT: Guard against availability of autoinit with something more cohesive than just __cplusplus
    ///
    /// @tparam autoinit
    /// @tparam T
    /// @param c
    /// @param err
    /// @param v
    /// @return true indicates processing is complete
    template <
#if __cplusplus >= 201103L
        bool autoinit = true,
#endif
        typename T>
    bool get(char_type c, ios_base::iostate& err, T& v)
    {
        switch(state_.state_)
        {
            case Start:
#if __cplusplus >= 201103L
                if(autoinit) v = 0;
#else
                v = 0;
#endif
                // DEBT: Revisit if we need to play with widening/narrowing/conversion
                // to ensure this hyphen compare is proper
                if (is_signed<T>() && c == ctype_type::widen('-'))
                {
                    state_.state_ = NominalNegative;
                    return false;
                }
                else
                    state_.state_ = NominalPositive;

#if __has_cpp_attribute(fallthrough)
                [[fallthrough]];
#endif

            case FirstPositive:
                if(nominal<true>(c, err, v))
                {
                    // "if the conversion function fails std::ios_base::failbit is assigned to err" [1]
                    err |= ios_base::failbit;
                    return true;
                }
                else
                    return false;

            case FirstNegative:
                if(nominal<false>(c, err, v))
                {
                    // "if the conversion function fails std::ios_base::failbit is assigned to err" [1]
                    err |= ios_base::failbit;
                    return true;
                }
                else
                    return false;

            case NominalPositive:
                return nominal<true>(c, err, v);

            case NominalNegative:
                return nominal<false>(c, err, v);

            case Overflow:
                // Merely consumed numbers in this mode
                return cbase_type::from_char(c).has_value();

            // FIX: Just to satisfy compilation
            default:
                return true;
        }
    }


    // Does NOT loop
    template <class Iter, class T>
    bool get(Iter& i, Iter end,
        ios_base::iostate& err, T& v)
    {
        if(i != end) return get(*i++, err, v);

        err |= ios_base::eofbit;
        return true;
    }

    // Just a bit of future proofing
    ESTD_CPP_CONSTEXPR_RET num_get(locale_type) {}
    ESTD_CPP_DEFAULT_CTOR(num_get)
};



template <typename Char, class Locale>
struct num_get<0, Char, Locale>
{
    union
    {
        num_get<8, Char, Locale> base8;
        num_get<10, Char, Locale> base10;
        num_get<16, Char, Locale> base16;
    };
};

template <typename TChar, class TLocale, bool boolalpha>
struct bool_get;

// numeric (non alpha) version
template <typename TChar, class TLocale>
struct bool_get<TChar, TLocale, false> : num_get<2, TChar, TLocale>
{
    typedef TChar char_type;
    //typedef num_get<2, char_type, TLocale> base_type;

    bool get(char_type c, ios_base::iostate& err, bool& v)
    {
        switch(c)
        {
            case '0':
                v = false;
                break;

            // Yes, it is the case that '1' is expected to appear on parse failure for bool
            // As per https://en.cppreference.com/w/cpp/locale/num_get/get 'Stage 3'
            default:
                err |= ios_base::failbit;
#if __has_cpp_attribute(fallthrough)
                [[fallthrough]];
#endif

            case '1':
                v = true;
                break;
        }

        // Numeric bool is always one character, so we're always done no matter
        // what the input
        return true;
    }

    template <class TIter>
    bool get(TIter& i, TIter end, ios_base::iostate& err, bool& v)
    {
        if(i != end) return get(*i++, err, v);

        err |= ios_base::eofbit;
        return true;
    }


    bool_get(TLocale l)
    //    : base_type(l)
    {}
    bool_get() {}
};

// alpha version
template <typename Char, class Locale>
struct bool_get<Char, Locale, true>
{
    typedef Char char_type;
    typedef Locale locale_type;
    typedef estd::numpunct<char_type, locale_type> numpunct_type;

    enum state
    {
        Start = 0
    };

    internal::chooser chooser;
    estd::layer2::basic_string<const char_type, 0> names[2];

    // NOTE: This method never sets eof bit
    bool get(char_type c, ios_base::iostate& err, bool& v)
    {
        if(chooser.process(names, c) == false)
            return false;

        if(chooser.chosen() != -1)
        {
            v = chooser.chosen() == 0;
        }
        else
        {
            v = false;
            err |= ios_base::failbit;
        }

        return true;
    }

    template <class TIter>
    bool get(TIter& i, TIter end, ios_base::iostate& err, bool& v)
    {
        if(i != end) return get(*i++, err, v);

        err |= ios_base::eofbit;
        return true;
    }


    void set_names(locale_type l)
    {
        estd::numpunct<char_type, locale_type> n = use_facet<numpunct<char> >(l);
        names[0] = n.truename();
        names[1] = n.falsename();
    }

#if __cplusplus >= 201103L
    // DEBT: Not instance-locale compat
    constexpr explicit bool_get(locale_type l) : names { numpunct_type::truename(), numpunct_type::falsename() }
    {}
    constexpr bool_get() : names { numpunct_type::truename(), numpunct_type::falsename() }
    {}
#else
    bool_get() { set_names(locale_type()); }
    bool_get(locale_type l) { set_names(l); }
#endif
    // { set_names(); }
};



}}

#include "../../macro/pop.h"
