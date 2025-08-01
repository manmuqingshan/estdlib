#pragma once

#include "internal/type_traits/is_trivial.h"

#include "internal/fwd/flags.h"

namespace estd { namespace detail { inline namespace v1 {

///
/// Designed really to be fully consteval friendly, but works as intended merely as constexpr
///
template <class Enum>
class flags
{
public:
#if FEATURE_ESTD_UNDERLYING_TYPE
    using int_type = typename estd::underlying_type<Enum>::type;
#else
    using int_type = int;
#endif

    using value_type = Enum;

private:
    value_type value_;

    // Force int conversion to always fail, we don't want 'bool' conversion to fool us in these cases
    //constexpr operator int_type() const { return value_; }

public:
    constexpr explicit flags(int_type v) :
        value_{value_type(v)}
    {}

    constexpr flags(const value_type& value) : value_{value}    {}

    ESTD_CPP_CONSTEXPR(14) flags& operator|=(flags v)
    {
        value_ |= v;
        return *this;
    }

    constexpr operator value_type() const { return value_; }

    constexpr operator bool() const { return value_ != value_type{}; }

    constexpr value_type value() const { return value_; }

    // Putting these all as members instead of freestanding operators for easy access to int_type

    constexpr flags operator~() const
    {
        return flags(~int_type(value_));
    }

    constexpr flags operator ^(value_type v) const
    {
        return flags{int_type(v) ^ int_type(value_)};
    }

    constexpr flags operator |(value_type v) const
    {
        return flags{int_type(v) | int_type(value_)};
    }

    constexpr flags operator &(value_type v) const
    {
        return flags{int_type(v) & int_type(value_)};
    }
};


template <class Enum>
constexpr bool operator==(const flags<Enum>& lhs, const Enum& rhs)
{
    return lhs.value() == rhs;
}


template <class Enum>
constexpr bool operator==(const flags<Enum>& lhs, const flags<Enum>& rhs)
{
    return lhs.value() == rhs.value();
}

template <class Enum>
constexpr bool operator!=(const flags<Enum>& lhs, const flags<Enum>& rhs)
{
    return lhs.value() != rhs.value();
}


}}}


// Auto-promotes 'Enum' to flags<Enum> during these operations
#define ESTD_FLAGS(Enum)    \
constexpr estd::detail::v1::flags<Enum> operator~(Enum v)    \
{ return ~estd::detail::v1::flags<Enum>(v); }     \
constexpr estd::detail::v1::flags<Enum> operator^(Enum lhs, Enum rhs)    \
{ return estd::detail::v1::flags<Enum>(lhs) ^ rhs; }     \
constexpr estd::detail::v1::flags<Enum> operator|(Enum lhs, Enum rhs)    \
{ return estd::detail::v1::flags<Enum>(lhs) | rhs; }     \
constexpr estd::detail::v1::flags<Enum> operator&(Enum lhs, Enum rhs)    \
{ return estd::detail::v1::flags<Enum>(lhs) & rhs; }

