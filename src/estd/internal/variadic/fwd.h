#pragma once

#include "../raw/cstddef.h"
#include "../raw/utility.h"

#if __cpp_variadic_templates
namespace estd { namespace internal {

template <size_t pos, class ...Types>
struct get_type_at_index;

// Very similar to std::variant_alternative
template <size_t index, class ...Types>
using type_at_index = typename get_type_at_index<index, Types...>::type;

// Plural of is_trivial
// DEBT: Consider putting out into main estd namespace
template <class ...TArgs>
struct are_trivial;

// Indicates the function/constructor expects a functor for iterating
// over all the variadic possibilities
struct in_place_visit_t : in_place_tag {};

// internal = specialization version, since std indicates it's undefined to specialize conjunction
template <class T>
struct conjunction;

}

namespace variadic {

namespace detail {

template <size_t size, class TEval, class ...Types>
struct selector;

template <size_t size, class TEval, class T, T ...Types>
struct value_selector;

}

template <class TEval, class ...Types>
using selector = typename detail::selector<sizeof...(Types), TEval, Types...>::selected;

template <class TEval, class ...Types>
using projector = typename detail::selector<sizeof...(Types), TEval, Types...>::projected;

inline namespace v1 {

template <size_t I, class T, T v>
struct visitor_value;

template <size_t I, class T>
struct visitor_index;

template <size_t I, class T>
struct visitor_instance;

}

inline namespace v2 {

template <size_t I, class T, T v>
using value = v1::visitor_value<I, T, v>;

template <size_t I, class T>
using type = v1::visitor_index<I, T>;

template <size_t I, class T>
using instance = v1::visitor_instance<I, T>;

}

// EXPERIMENTAL at this time
namespace v3 {

template <class T, size_t I = 1000>
using instance = v1::visitor_instance<I, T>;

}

template <class ...Types>
struct types;

template <typename T, T ...Is>
struct values;

#if __cpp_template_auto
namespace experimental {

template <auto ...Is>
struct v;

template <auto T, decltype(T) ...Is>
using v2 = values<decltype(T), T, Is...>;


}
#endif

template <class T, bool v = true>
struct projected_result;

template <class T, T ...Values>
struct value_visitor;

template <class ...Types>
struct type_visitor;

}

template <class ...Types>
using type_sequence = variadic::types<Types...>;

}
#else
namespace estd {

namespace internal {

template <size_t index, class T1, class T2, class T3>
struct get_type_at_index;

template <class T1, class T2, class T3>
struct are_trivial;

}

}
#endif
