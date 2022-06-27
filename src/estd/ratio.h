#pragma once

#include "internal/platform.h"
#include "internal/numeric.h"
#include "type_traits.h"
#include "cstdint.h"

namespace estd {

template<
    std::intmax_t Num,
    std::intmax_t Denom = 1
> class ratio
{
private:
    static CONSTEXPR std::intmax_t gcd = internal::gcd<Num, Denom>::value;
public:
    /// Reduced version
    typedef ratio<Num / gcd, Denom / gcd> type;

    static CONSTEXPR std::intmax_t num = Num;
    static CONSTEXPR std::intmax_t den = Denom;
};

typedef ratio<1, 1000000000000> pico;
typedef ratio<1, 1000000000> nano;
typedef ratio<1, 1000000> micro;
typedef ratio<1, 1000> milli;
typedef ratio<1, 100> centi;
typedef ratio<1, 10> deci;
typedef ratio<10, 1> deca;
typedef ratio<1000, 1> kilo;
typedef ratio<1000000, 1> mega;
typedef ratio<1000000000, 1> giga;
typedef ratio<1000000000000, 1> tera;

namespace detail {

template <class R1, class R2>
struct ratio_add;

template <class R1, class R2>
struct ratio_divide;

template <class R1, class R2>
struct ratio_multiply;

}

/*
// both have numerator of 1
template<std::intmax_t Denom1, std::intmax_t Denom2>
struct ratio_divide<ratio<1, Denom1>, ratio<1, Denom2> >
{
    static CONSTEXPR std::intmax_t num = Denom2;
    static CONSTEXPR std::intmax_t den = Denom1;
}; */

// TODO: Make a beefier version which queries numeric_limits

/*
// second has a numerator of 1, first has variable numerator
template<std::intmax_t Num1, std::intmax_t Denom1, std::intmax_t Denom2>
struct ratio_divide<ratio<Num1, Denom1>, ratio<1, Denom2> >
{
    static CONSTEXPR std::intmax_t num = Denom2 * Num1;
    static CONSTEXPR std::intmax_t den = Denom1;
}; */

/*
// second has a numerator of 1, first has variable numerator
template<std::intmax_t Num1, std::intmax_t Denom1, std::intmax_t Denom2,
         class = typename estd::enable_if<
             //((Denom2 * Num1) % Denom1) == 0
             true
             > >
struct ratio_divide<ratio<Num1, Denom1>, ratio<1, Denom2> >
{
    static CONSTEXPR std::intmax_t num = Denom2 * Num1;
    static CONSTEXPR std::intmax_t den = Denom1;
}; */

namespace detail {

// https://en.cppreference.com/w/cpp/numeric/ratio/ratio_divide indicates no overflow
// processing is required
template <std::intmax_t Num1, std::intmax_t Num2,
    std::intmax_t Denom1, std::intmax_t Denom2>
struct ratio_divide<ratio<Num1, Denom1>, ratio<Num2, Denom2> >
{
private:
    static CONSTEXPR std::intmax_t gcd =
        internal::gcd<Denom2 * Num1, Denom1 * Num2>::value;

public:
    static CONSTEXPR std::intmax_t num = Denom2 * Num1 / gcd;
    static CONSTEXPR std::intmax_t den = Denom1 * Num2 / gcd;

    typedef typename estd::ratio<num, den>::type type;
};


template <std::intmax_t Num1, std::intmax_t Num2,
    std::intmax_t Denom1, std::intmax_t Denom2>
struct ratio_multiply<ratio<Num1, Denom1>, ratio<Num2, Denom2> >
{
private:
    static CONSTEXPR std::intmax_t gcd =
        internal::gcd<Denom2 * Denom1, Num1 * Num2>::value;

public:
    static CONSTEXPR std::intmax_t num = Num1 * Num2 / gcd;
    static CONSTEXPR std::intmax_t den = Denom1 * Denom2 / gcd;

    typedef typename estd::ratio<num, den>::type type;
};


template <std::intmax_t Num1, std::intmax_t Num2,
    std::intmax_t Denom1, std::intmax_t Denom2>
struct ratio_add<ratio<Num1, Denom1>, ratio<Num2, Denom2> >
{
private:
    typedef estd::ratio<Num1 * Denom2, Denom1 * Denom2> normalized1;
    typedef estd::ratio<Num2 * Denom1, Denom1 * Denom2> normalized2;

public:
    static CONSTEXPR std::intmax_t num = normalized1::num + normalized2::num;
    static CONSTEXPR std::intmax_t den = normalized1::den;

    typedef typename estd::ratio<num, den>::type type;
};



}

#ifdef FEATURE_CPP_ALIASTEMPLATE
template <class R1, class R2>
using ratio_multiply = typename detail::ratio_multiply<R1, R2>::type;

template <class R1, class R2>
using ratio_divide = typename detail::ratio_divide<R1, R2>::type;

template <class R1, class R2>
using ratio_add = typename detail::ratio_add<R1, R2>::type;
#endif

}
