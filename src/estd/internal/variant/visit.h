#pragma once

#include "../fwd/variant.h"
#include "../variadic/type_sequence.h"

#include "accessors.h"

namespace estd { namespace internal {


template <class R, class Visitor, class ...Types>
struct variant_visit
{
    static constexpr R visit(Visitor&& visitor, variant<Types...>& vv)
    {
        using return_type = R;
        return_type result;
        int counter = 0;
        const int index = vv.index();
        bool found = ((counter++ == index ? (result = visitor(get<Types>(vv)), true) : false) || ...);
        return result;
    }
};

template <class Visitor, class ...Types>
struct variant_visit<void, Visitor, Types...>
{
    static constexpr void visit(Visitor&& visitor, variant<Types...>& vv)
    {
        //using intseq = index_sequence_for<Types...>;
        int counter = 0;
        const int index = vv.index();
        bool found = ((counter++ == index ? (visitor(estd::get<Types>(vv)), true) : false) || ...);
    }

};


template <class Visitor, class ...Types, class R>
constexpr R visit(Visitor&& visitor, variant<Types...>& vv)
{
    return variant_visit<R, Visitor, Types...>::visit(std::forward<Visitor>(visitor), vv);
}

}}
