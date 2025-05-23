/**
 * @file
 * standalone implementation so that can be #included from support_platform.h
 * BE CAREFUL - including this directly may not process FEATURE flag right
 * (see README.md)
 */
#pragma once

#include "../macros.h"

namespace estd {

#ifdef FEATURE_CPP_ENUM_CLASS
enum class byte : unsigned char {};
#else
// non-member operator overloads want a full struct or enum
struct byte
{
    unsigned char value;

    byte() {}

    byte(unsigned char value) : value(value) {}

    operator unsigned char() const { return value; }
};
#endif

// TODO: Only allow overloads which conform to is_integral, as per spec
// TODO: Optimize by doing direct unsigned char bit in-place operations
// (i.e. |= doesn't devolve into l | r) - make sure we don't violate aliasing
// rules in the process

template <class IntegerType>
//#if defined(FEATURE_CPP_DEDUCE_RETURN) and defined(FEATURE_CPP_CONSTEXPR)
//constexpr auto to_integer(byte b) noexcept -> is_integral<IntegerType>::
//#else
ESTD_CPP_CONSTEXPR_RET IntegerType to_integer(byte b) NOEXCEPT
//#endif
{
    //is_integral<IntegerType>::value;
    return IntegerType(b);
}


// NONSTANDARD
// Because most of us aren't running C++17 yet with relaxed rules to do the
// canonical byte v={0} or similar, so do this in the meantime
template <class IntegerType>
ESTD_CPP_CONSTEXPR_RET byte to_byte(IntegerType value) NOEXCEPT
{
#ifdef FEATURE_CPP_ENUM_CLASS
    return (byte) value;
#else
    return byte(value);
#endif
}


template <class IntegerType>
ESTD_CPP_CONSTEXPR_RET byte operator <<(byte b, IntegerType shift) NOEXCEPT
{
    return (byte)((unsigned char)b << shift);
    //return b;
}

template <class IntegerType>
ESTD_CPP_CONSTEXPR_RET byte& operator >>=(byte& b, IntegerType shift) NOEXCEPT
{
    //return ((unsigned char&)b) >>= shift;
    return b = (byte)(((unsigned char)b) >> shift);
    //return b;
}


template <class IntegerType>
ESTD_CPP_CONSTEXPR_RET byte& operator <<=(byte& b, IntegerType shift) NOEXCEPT
{
    return b = b << shift;
    //return b = (byte)(((unsigned char)b) << shift);
    //return b;
}


ESTD_CPP_CONSTEXPR_RET byte operator|(byte l, byte r) NOEXCEPT
{
    return to_byte(static_cast<unsigned int>(l) | static_cast<unsigned int>(r));
}

#if __cpp_constexpr >= 201304L  // For relaxed constraints so we can change 'l'
constexpr
#else
inline
#endif
byte& operator|=(byte& l, byte r) NOEXCEPT
{
    return l = l | r;
}


}

#ifdef FEATURE_STD_CSTDDEF
#include <cstddef>

#if __cplusplus < 201103L
namespace std {

typedef void* nullptr_t;

}
#endif

#else
#include <stddef.h>

namespace std {

typedef ::size_t size_t;
typedef ::ptrdiff_t ptrdiff_t;

// DEBT: Not sure what it is about AVR which excludes a ton of things
// but it would be better to ascertain that and flag that rather than AVR itself
#if ESTD_MCU_ATMEL_AVR
typedef ::nullptr_t nullptr_t;
#endif

}
#endif


namespace estd {

using std::size_t;
using std::ptrdiff_t;
using std::nullptr_t;

}
