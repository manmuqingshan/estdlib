#pragma once

#include "../../allocators/handle_desc.h"

namespace estd { namespace internal { namespace impl {


template <class TAllocator, class Policy = monostate>
struct allocated_array : estd::handle_descriptor<TAllocator>
{
    typedef estd::handle_descriptor<TAllocator> base_type;
    typedef base_type base_t;
    typedef typename base_type::allocator_type allocator_type;
    typedef typename base_type::allocator_traits allocator_traits;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;
    typedef Policy policy_type;

    typedef typename base_type::size_type size_type;

    ESTD_CPP_CONSTEXPR_RET size_type max_size() const
    { return base_type::get_allocator().max_size(); }

    ESTD_CPP_DEFAULT_CTOR(allocated_array)

    ESTD_CPP_FORWARDING_CTOR(allocated_array)

    handle_with_offset offset(size_type pos) const
    {
        return base_type::get_allocator().offset(base_t::handle(), pos);
    }
};


}}}
