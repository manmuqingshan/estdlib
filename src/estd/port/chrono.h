#pragma once

#include "../internal/chrono.h"
#include "../ratio.h"
#include "../limits.h"

// FEATURE_STD_CHRONO_CORE includes durations, time_points and implicitly ratios
// FEATURE_STD_CHRONO_CLOCK includes system_clock and friends
#if (defined(FEATURE_POSIX_CHRONO) || defined(ESTD_SDK_IDF) || defined(LIB_PICO_STDLIB)) && !defined(FEATURE_ESTD_NATIVE_CHRONO) && __cplusplus >= 201103L
// DEBT: Doing this define here is the wrong spot - should be earlier in port/platform chain
#define FEATURE_STD_CHRONO
#define FEATURE_STD_CHRONO_CORE 1
#if !defined(LIB_PICO_STDLIB)
// As per https://github.com/raspberrypi/pico-sdk/issues/1034 we exclude rpi pico
#define FEATURE_STD_CHRONO_CLOCK 1
#endif
#include <chrono>
#endif

#ifdef FEATURE_PRAGMA_PUSH_MACRO
#pragma push_macro("abs")
#pragma push_macro("max")
#pragma push_macro("min")
#undef abs
#undef max
#undef min
#endif

// DEBT: Move this, does not belong in 'port'

#ifdef FEATURE_ESTD_CHRONO

namespace estd { namespace chrono {

template <class Rep>
struct duration_values
{
    static ESTD_CPP_CONSTEXPR_RET Rep zero() { return Rep(0); }
    static ESTD_CPP_CONSTEXPR_RET Rep min() { return estd::numeric_limits<Rep>::min(); }
    static ESTD_CPP_CONSTEXPR_RET Rep max() { return estd::numeric_limits<Rep>::max(); }
};

template<
        class Rep,
        class Period
>
class duration
{
    // confusingly, 'ticks' actually represents # of periods, not specifically
    // system ticks
    Rep ticks;

protected:
    template <class Rep2, class Period2>
    static ESTD_CPP_CONSTEXPR_RET Rep convert_from(const duration<Rep2, Period2>& d);

#if FEATURE_STD_CHRONO_CORE
    template <class Rep2, class Period2>
    static ESTD_CPP_CONSTEXPR_RET Rep convert_from(const std::chrono::duration<Rep2, Period2>& d);
#endif

public:
    typedef Rep rep;
    typedef typename Period::type period;

    ESTD_CPP_CONSTEXPR_RET rep count() const { return ticks; }

    ESTD_CPP_DEFAULT_CTOR(duration)

    template <class Rep2>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    explicit duration(const Rep2& r) : ticks(r)
    {

    }

    template <class Rep2, class Period2>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    duration(const duration<Rep2, Period2>& d);

#if FEATURE_STD_CHRONO_CORE
    template <class Rep2, class Period2>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    duration(const std::chrono::duration<Rep2, Period2>& d) :
        ticks(convert_from(d))
    {}

    typedef std::ratio<Period::num, Period::den> std_period_type;

    operator std::chrono::duration<Rep, std_period_type>() const
    {
        std::chrono::duration<Rep, std_period_type> converted(count());

        return converted;
    }
#endif

    duration& operator+=(const duration& d)
    {
        ticks += d.ticks;
        return *this;
    }


    duration& operator-=(const duration& d)
    {
        ticks -= d.ticks;
        return *this;
    }

    duration& operator*=(const rep& r)
    {
        ticks *= r;
        return *this;
    }

    CONSTEXPR duration operator-() const
    {
#ifdef FEATURE_CPP_STATIC_ASSERT
        static_assert (numeric_limits<rep>::is_signed, "operator -() requires a signed Rep type");
#endif

        return duration(-ticks);
    }

    CONSTEXPR duration operator+() const { return *this; }

    static CONSTEXPR duration min() { return duration(duration_values<rep>::min()); }
    static CONSTEXPR duration max() { return duration(duration_values<rep>::max()); }
    static CONSTEXPR duration zero() { return duration(duration_values<rep>::zero()); }
};

#ifdef FEATURE_STD_CHRONO
template <class ToDuration, class Rep, class Period>
inline ToDuration duration_cast(const std::chrono::duration<Rep, Period>& d)
{
    typedef ratio<Period::num, Period::den> period_type;

    duration<Rep, period_type> our_d(d);

    return duration_cast<ToDuration>(our_d);
}
#endif

#endif // FEATURE_ESTD_CHRONO

// These lower-precision ones are available even during non-FEATURE_ESTD_CHRONO
typedef internal::estd_chrono::duration<internal::nano_rep, nano> nanoseconds;
typedef internal::estd_chrono::duration<internal::micro_rep, micro> microseconds;
typedef internal::estd_chrono::duration<internal::milli_rep, milli> milliseconds;
typedef internal::estd_chrono::duration<internal::seconds_rep> seconds;
typedef internal::estd_chrono::duration<internal::minutes_rep, ratio<60> > minutes;
typedef internal::estd_chrono::duration<internal::hours_rep, ratio<3600> > hours;

// NOTE: AVR compiler requires this long typecast.  Doesn't hurt anything (though it's
// a bit ugly)
typedef internal::estd_chrono::duration<internal::days_rep, ratio<(long)3600 * 24> > days;
typedef internal::estd_chrono::duration<internal::weeks_rep, ratio<(long)3600 * 24 * 7> > weeks;

// "Each of the predefined duration types days, weeks, months and years covers a range
//  of at least ±40000 years. years is equal to 365.2425 days (the average length of a
//  Gregorian year). months is equal to 30.436875 days (exactly 1/12 of years)."
// https://en.cppreference.com/w/cpp/chrono/duration
typedef internal::estd_chrono::duration<internal::months_rep, ratio<(long)2629746> > months;
typedef internal::estd_chrono::duration<internal::years_rep, ratio<(long)31556952> > years;

#ifdef FEATURE_ESTD_CHRONO

template<
    class Clock,
    class Duration
> class time_point
{
    Duration m_time_since_epoch;

public:
    typedef Clock clock;
    typedef Duration duration;
    typedef typename duration::rep rep;
    typedef typename duration::period period;
    typedef estd::chrono::internal::clock_traits<clock> clock_traits;

    // NOTE: *may* deviate from spec.  Leaves m_time_since_epoch undefined
    // spec, to my ears, is unclear:
    //   "Default constructor, creates a time_point with a value of Clock's epoch."
    // this implies a 'now' operation, but I am not sure.  Since we are targetting embedded,
    // less is more, so swing vote goes to doing nothing here.  This turns out to be helpful,
    // now one can leave a time_point as a global variable without worring about startup-time
    // init issues
    // NOTE: considering above, I am interpreting 'value of Clock's epoch' to generally
    // mean 0 i.e. the starting point from which the clock begins - unix epoch is around 1970,
    // and the seconds count up from there so 0 = 1970.  The inspecific nature of things
    // implies there's a way to reach into Clock itself and grab this epoch
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    time_point() : m_time_since_epoch(0) {}

#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    explicit time_point(const Duration& duration) : m_time_since_epoch(duration) {}

    // NOTE: Compiles, but not tested
    template <class TDuration2>
    time_point(const time_point<Clock, TDuration2>& t) :
        m_time_since_epoch(t.time_since_epoch())
    {}

#ifdef FEATURE_STD_CHRONO
    // Deviates from spec, normally one is not permitted to convert time points between clocks.
    // We make an exception here since one might want to convert from
    // std::chrono::time_point to estd::chrono::time_point
    // NOTE: Have to pull TClock3 shenanigan, otherwise compiler gets upset
    // claiming we're specializing clock_traits after instantiation
    // DEBT: What we really want to do is compare clock tag type itself and make
    // sure it matches - even if it's not unix_epoch_clock_tag.  Very much an edge case though
    // DEBT: Probably should do https://en.cppreference.com/w/cpp/chrono/clock_cast instead
    template <class TClock2, class TDuration2, class TClock3 = Clock,
        typename estd::enable_if<
            estd::is_base_of<
                estd::chrono::internal::unix_epoch_clock_tag,
                estd::chrono::internal::clock_traits<TClock2> >::value
                &&
            estd::is_base_of<
                estd::chrono::internal::unix_epoch_clock_tag,
                //clock_traits
                //estd::internal::clock_traits<Clock>
                estd::chrono::internal::clock_traits<TClock3>
                >::value
            , bool>::type = true>
    inline time_point(const std::chrono::time_point<TClock2, TDuration2>& t) :
        m_time_since_epoch(t.time_since_epoch())
    {}
#endif

    Duration time_since_epoch() const { return m_time_since_epoch; }


    //constexpr
    time_point& operator+=( const duration& d )
    {
        m_time_since_epoch += d;
        return *this;
    }

    time_point& operator-=( const duration& d )
    {
        m_time_since_epoch -= d;
        return *this;
    }

    // DEBT: Spec calls for this operation as outside of class
    time_point operator+(const duration& d)
    {
        time_point copied(*this);
        copied += d;
        return copied;
    }

    // DEBT: Spec calls for this operation as outside of class
    time_point operator-(const duration& d)
    {
        time_point copied(*this);
        copied -= d;
        return copied;
    }

    static inline CONSTEXPR time_point min() NOEXCEPT
    {
        return time_point(duration::min());
    }
};


}


#ifdef FEATURE_CPP_INLINE_NAMESPACE
inline
#endif
namespace literals {

#ifdef FEATURE_CPP_INLINE_NAMESPACE
inline
#endif
namespace chrono_literals {

// CLang allows this during the compile phase, but ignores the definitions and generates
// a bunch of warnings.  Until we can repair that, CLang is disabled.  In the short term,
// we only enable these literals for GCC
#if defined(__GNUC__) && !defined(__clang__)
#define FEATURE_ESTD_CHRONO_LITERALS 1

// Compiler complains that these are in the reserved suffix category, which is in a sense
// true - i.e. an actual std::literal::chrono_literals ""s is "correct", but we like to use
// our own version of seconds, microseconds, etc.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"

#ifdef FEATURE_CPP_USER_LITERAL
constexpr chrono::seconds operator "" s(unsigned long long s)
{
    return chrono::seconds(s);
}

constexpr chrono::milliseconds operator "" ms(unsigned long long ms)
{
    return chrono::milliseconds(ms);
}

constexpr chrono::microseconds operator "" us(unsigned long long rep)
{
    return chrono::microseconds(rep);
}

constexpr chrono::nanoseconds operator "" ns(unsigned long long rep)
{
    return chrono::nanoseconds(rep);
}

#endif

#pragma GCC diagnostic pop
#else
#define FEATURE_ESTD_CHRONO_LITERALS 0
#endif
}

}


}

#include "chrono.hpp"

#ifdef FEATURE_PRAGMA_PUSH_MACRO
#pragma pop_macro("min")
#pragma pop_macro("max")
#pragma pop_macro("abs")
#endif


#endif