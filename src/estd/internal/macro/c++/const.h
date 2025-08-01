#pragma once

// Take pages out of GCC playbook
#if __cplusplus >= 202002L
#define ESTD_CPP20_CONSTEXPR constexpr
#else
#define ESTD_CPP20_CONSTEXPR
#endif

#if __cplusplus >= 201703L
#define ESTD_CPP17_CONSTEXPR constexpr
#else
#define ESTD_CPP17_CONSTEXPR
#endif

#if __cpp_constexpr >= 201304L
#define ESTD_CPP14_CONSTEXPR constexpr
#else
#define ESTD_CPP14_CONSTEXPR
#endif

#if __cplusplus >= 201103L
#define ESTD_CPP11_CONSTEXPR constexpr
#else
#define ESTD_CPP11_CONSTEXPR
#endif

#define ESTD_CPP_CONSTEXPR(v)    ESTD_CPP ## v ## _CONSTEXPR

// 23FEB25 MB DEBT: Inconsistency between ESTD_CPP_ prefix and lack of prefix
#ifdef __cpp_conditional_explicit
#define CONSTEXPR_EXPLICIT(conditional) constexpr explicit(conditional)
#elif __cpp_constexpr
#define CONSTEXPR_EXPLICIT(conditional) constexpr explicit
#else
#define CONSTEXPR_EXPLICIT(conditional) inline
#endif

#if __cplusplus >= 201703L
#define ESTD_CPP_IF_CONSTEXPR if constexpr
#else
#define ESTD_CPP_IF_CONSTEXPR if
#endif

// DEBT: Perhaps we can merely call this CONSTEVAL
#if __cpp_consteval
#define ESTD_CPP_CONSTEVAL consteval
#else
#define ESTD_CPP_CONSTEVAL constexpr
#endif

