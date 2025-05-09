#pragma once

#include <avr/pgmspace.h>

#include <estd/internal/fwd/variant.h>
#include <estd/internal/dynamic_array.h>
#include <estd/internal/container/locking_accessor.h>
#include <estd/internal/container/iterator.h>

#include "accessor.h"

#include "../../internal/macro/push.h"

namespace estd {

namespace internal {

namespace impl {

// NOTE: None of the underlying lock mechanisms are gonna work here, so
// we might be better off with a pure manual allocator like we started
// with with pgm_allocator (now in obsolete area)
template <class T, size_t N>
struct pgm_allocator2 : estd::layer2::allocator<const T, N>
{
    using base_type = estd::layer2::allocator<const T, N>;

    ESTD_CPP_FORWARDING_CTOR(pgm_allocator2)

    // DEBT: Kinda sloppy, exposing this this way, but it gets the job done
    // - Not deriving from pgn_allocator2 itself because then we get ambiguities
    // for things like const_pointer
    ESTD_CPP_CONSTEXPR_RET typename base_type::const_pointer data(
        typename base_type::size_type offset = 0) const
    {
        return base_type::data(offset);
    }

    /* Basically ready, just need to move pgm_accessor2 upwards
     * so this can get access
    using iterator = estd::internal::locking_iterator<
        pgm_allocator2, pgm_accessor2<T>,
        estd::internal::locking_iterator_modes::ro >;   */
};


// Just like pgm_allocator2, but from layer1 not layer2
/*
template <class T, size_t N>
struct layer1_pgm_allocator : estd::layer1::allocator<const T, N>
{
    using base_type = estd::layer1::allocator<const T, N>;

    ESTD_CPP_FORWARDING_CTOR(layer1_pgm_allocator)

    // DEBT: See pgm_allocator2
    ESTD_CPP_CONSTEXPR_RET typename base_type::const_pointer data(
        typename base_type::size_type offset = 0) const
    {
        return base_type::data(offset);
    }
}; */
template <class T, size_t N>
using layer1_pgm_allocator = estd::internal::single_fixedbuf_allocator<T, N>;




template <class T, size_t N, class Allocator>
struct pgm_allocator_traits_base
{
    using value_type = add_const_t<T>;
    using pointer = const PROGMEM value_type*;
    using const_pointer = pointer;
    using handle_type = pointer;
    using handle_with_offset = pointer;
    using size_type = uint16_t;
    using allocator_type = Allocator;

    static CONSTEXPR bool is_stateful_exp = false;
    static CONSTEXPR bool is_locking_exp = false;

    using allocator_valref = allocator_type;;
    using iterator = estd::internal::locking_iterator<
        allocator_type,
        pgm_accessor2<T>,
        estd::internal::locking_iterator_modes::ro>;

    // NOTE: Tricky behavior - by NOT exposing this, dynamic_array_helper 
    // specialization can't specialize ON it, opening doorway to specializing
    // on Impl instead
    //static constexpr estd::internal::allocator_locking_preference::_
    //    locking_preference = internal::allocator_locking_preference::iterator;
};

// DEBT: Do this with a using
template <class T, size_t N>
struct pgm_allocator_traits : pgm_allocator_traits_base<T, N, pgm_allocator2<T, N> > {};

template <class T, size_t N>
using layer1_pgm_allocator_traits = pgm_allocator_traits_base<T, N, layer1_pgm_allocator<T, N> >;


}   // estd::internal::impl

}   // estd::internal

}

#include "../../internal/macro/pop.h"
