#pragma once

#include "internal/cctype.h"
#include "internal/locale/ctype.h"

// DEBT: Revamp all these to use locale's ctype
namespace estd {

constexpr int isupper(int ch)
{
    return internal::ascii_isupper(ch);
}

inline int islower(int ch)
{
    return 'a' <= ch && ch <= 'z';
}

inline int isalpha(int ch)
{
    return isupper(ch) || islower(ch);
}

inline ESTD_CPP_CONSTEXPR(14) int isspace(int ch)
{
    return internal::ascii_isspace(ch);
}

inline constexpr int isdigit(int ch)
{
    return internal::ascii_isdigit(ch);
}

}
