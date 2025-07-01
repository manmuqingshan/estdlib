#pragma once

#include "internal/platform.h"

namespace estd { namespace numbers {

#if __cpp_constexpr >= 201304L
// DEBT: Concerned these will round down at least significant digit if precision loss is encountered.
// So far, testing allays these concerns (e_v rounds up at least significant digit).  Keep eyes open.

template <class T>
constexpr T e_v = T(2.718281828459045235360287471352L);

// Thank you https://www2.cs.arizona.edu/icon/oddsends/phi.htm
template <class T>
constexpr T phi_v = T(1.6180339887498948482045868L);

template <class T>
constexpr T pi_v = T(3.1415926535897932385L);

// Thank you https://www.mathsisfun.com/scientific-calculator.html
template <class T>
constexpr T sqrt2_v = T(1.4142135623730950488L);

inline constexpr auto e = e_v<double>;
inline constexpr auto phi = phi_v<double>;
inline constexpr auto pi = pi_v<double>;
inline constexpr auto sqrt2 = sqrt2_v<double>;

// EXPERIMENTAL
#define __estd_lib_math_constants   201304L

#endif

}}

