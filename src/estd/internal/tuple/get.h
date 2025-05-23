#pragma once

#include "../fwd/tuple.h"
#include "../variadic/type_sequence.h"

namespace estd {

namespace internal {

// 'GetImpl' lifted and adapted from https://gist.github.com/IvanVergiliev/9639530
// TODO: use new variadic support area instead of this

template<bool sparse, unsigned index, typename First, typename... Rest>
struct GetImpl : GetImpl<sparse, index - 1, Rest...>
{
    /*
    typedef GetImpl<index - 1, Rest...> base_type;

    static typename base_type::const_valref_type value(const tuple<First, Rest...>& t)
    {
        return base_type::value(t);
    }

    static typename base_type::valref_type value(tuple<First, Rest...>& t)
    {
        return base_type::value(t);
    }   */
};

// cascades down eventually to this particular specialization
// to retrieve the 'first' item
template<bool sparse, typename First, typename... Rest>
struct GetImpl<sparse, 0, First, Rest...>
{
    //typedef First first_type;
    //using tuple_type = tuple<First, Rest...>;
    using target = tuple_storage<sparse, First>;
    using valref_type = typename target::valref_type;
    using const_valref_type = typename target::const_valref_type;

    static constexpr const_valref_type value(const tuple<sparse, First, Rest...>& t)
    {
        return t.first();
    }

    static valref_type value(tuple<sparse, First, Rest...>& t)
    {
        return t.first();
    }

    static First&& value(tuple<sparse, First, Rest...>&& t)
    {
        return std::forward<First>(t.first());
    }
};


}

template<int index, bool sparse, typename... Types>
constexpr typename tuple_element<index, tuple<Types...> >::const_valref_type get(
    const internal::tuple<sparse, Types...>& t)
{
    return internal::GetImpl<sparse, index, Types...>::value(t);
}

template<int index, bool sparse, typename... Types>
ESTD_CPP_CONSTEXPR(20) typename tuple_element<index, tuple<Types...> >::valref_type get(
    internal::tuple<sparse, Types...>& t)
{
    return internal::GetImpl<sparse, index, Types...>::value(t);
}

template<int index, bool sparse, typename... Types>
ESTD_CPP_CONSTEXPR(20) tuple_element_t<index, tuple<Types...> >&& get(internal::tuple<sparse, Types...>&& t)
{
    return internal::GetImpl<sparse, index, Types...>::value(
        std::forward<internal::tuple<sparse, Types...>>(t));
}

template<int index, bool sparse, typename... Types>
constexpr tuple_element_t<index, tuple<Types...> >&& get(const internal::tuple<sparse, Types...>&& t)
{
    return internal::GetImpl<sparse, index, Types...>::value(
        std::forward<const internal::tuple<sparse, Types...>>(t));
}

template <class T, bool sparse, typename... Types>
ESTD_CPP_CONSTEXPR(14) T& get(internal::tuple<sparse, Types...>& t)
{
    return get<internal::single_index_of_type<T, Types...>::index>(t);
}


template <class T, bool sparse, typename... Types>
ESTD_CPP_CONSTEXPR(14) T&& get(internal::tuple<sparse, Types...>&& t)
{
    return get<internal::single_index_of_type<T, Types...>::index>(t);
}

template <class T, bool sparse, typename... Types>
constexpr const T& get(const internal::tuple<sparse, Types...>& t)
{
    return get<internal::single_index_of_type<T, Types...>::index>(t);
}

template <class T, bool sparse, typename... Types>
constexpr const T&& get(const internal::tuple<sparse, Types...>&& t)
{
    return get<internal::single_index_of_type<T, Types...>::index>(t);
}

}
