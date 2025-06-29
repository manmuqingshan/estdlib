#pragma once

#include "../fwd/variant.h"
#include "../variadic/type_sequence.h"

#include "accessors.h"

#if __cplusplus >= 201703L
namespace estd { namespace internal {

// NOTE: Could have done constexpr if for different R types.  Specialization felt more
// natural, and we're prepped for c++11 compatibility (lean on estd::variadic in that case)

template <class R>
struct variant_visit
{
    template <class Visitor, class Variant, size_t ...Is>
    static constexpr R visit(Visitor &&visitor, Variant &vv, index_sequence<Is...>, bool* found)
    {
        using return_type = R;
        return_type result;
        const int index = vv.index();
        *found = ((Is == index ? (result = visitor(*get_ll<Is>(vv)), true) : false) || ...);
        return result;
    }
};

template <>
struct variant_visit<void>
{
    template <class Visitor, class Variant, size_t ...Is>
    static constexpr void visit(Visitor &&visitor, Variant &vv, index_sequence<Is...>, bool* found)
    {
        const int index = vv.index();
        *found = ((Is == index ? (visitor(*get_ll<Is>(vv)), true) : false) || ...);
    }
};

}

template <class Visitor, class ...Types, class R>
constexpr R visit(Visitor&& visitor, variant<Types...>& vv)
{
    [[maybe_unused]] bool found;
    using indices = index_sequence_for<Types...>;
    return internal::variant_visit<R>::visit(
        std::forward<Visitor>(visitor), vv, indices{}, &found);
}

}

#endif