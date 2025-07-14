#pragma once

namespace estd { namespace internal {

template <class Char>
inline constexpr bool ascii_isupper(Char ch)
{
    return 'A' <= ch && ch <= 'Z';
}

inline constexpr int ascii_isdigit(int ch)
{
    return '0' <= ch && ch <= '9';
}

// As per http://en.cppreference.com/w/cpp/string/byte/isspace
template <class Char>
ESTD_CPP_CONSTEXPR(14) bool ascii_isspace(Char ch)
{
    switch(ch)
    {
        case ' ':
        case 13:
        case 10:
        case '\f':
        case '\t':
        case '\v':
            return true;

        default:
            return false;
    }
}


}}