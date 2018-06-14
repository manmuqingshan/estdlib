#pragma once

#include "../type_traits.h"
#include "../traits/allocator_traits.h"

namespace estd { namespace internal {

// TODO: Utilize allocator_traits throughout
// TODO: See if there's a smooth way to incorporate initial
//       offset in here as well or otherwise incorporate gracefully with handle_with_offset

// handle_descriptor provides rich data about a memory handle:
//  - associated allocator
//  - size of allocated handle
// handle_descriptor seeks to describe already-allocated handles and as such doesn't attempt to
// auto allocate/construct/free/destroy anything
//
// is_stateful = is this a stateful (instance) allocator or a global (singleton-style) allocator
// has_size = does this allocator track size of the allocated item
// is_singular = standard allocator behavior is to be able to allocate multiple items.  a singular allocator can only allocate one item
template <class TAllocator, bool is_stateful, bool has_size, bool is_singular>
class handle_descriptor_base;

namespace impl {


template <class TValue, bool is_present, TValue default_value>
class value_evaporator;

template <class TValue, TValue default_value>
class value_evaporator<TValue, true, default_value>
{

};

template <class TValue, TValue default_value>
class value_evaporator<TValue, false, default_value>
{
protected:
};

// https://en.cppreference.com/w/cpp/language/ebo we can have specialized base classes which are empty
// and don't hurt our sizing
template <class TAllocator, bool is_stateful>
class allocator_descriptor_base;


template <class TAllocator>
class allocator_descriptor_base<TAllocator, true>
{
    TAllocator allocator;

public:
    typedef typename remove_reference<TAllocator>::type allocator_type;

    allocator_type& get_allocator() { return allocator; }
};


template <class TAllocator>
struct allocator_descriptor_base<TAllocator, false>
{
    typedef TAllocator allocator_type;

    allocator_type& get_allocator() { return TAllocator(); }
};

// singular technically doesn't track a handle
template <class TAllocator, bool is_singular>
class handle_descriptor_base;

template <class TAllocator>
class handle_descriptor_base<TAllocator, true>
{
protected:
    void set_handle(bool) {}

public:
    bool get_handle() const { return true; }
};



}

// singular without implicit size knowledge
// singular allocators have a simplified handle model, basically true = successful/good handle
// false = bad/unallocated handle - we assume always good handle for this descriptor, so no
// handle is actually tracked
template <class TAllocator, bool is_stateful>
class handle_descriptor_base<TAllocator, is_stateful, false, true> :
        impl::allocator_descriptor_base<TAllocator, is_stateful>
{
    typedef impl::allocator_descriptor_base<TAllocator, true> base_t;

public:
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;
    typedef allocator_traits<allocator_type> allocator_traits;

private:
    size_type m_size;

public:
    handle_descriptor_base(size_type initial_size = 0) : m_size(initial_size) {}

    size_type size() const { return m_size; }

    value_type& lock(size_type pos = 0, size_type count = 0)
    {
        return base_t::get_allocator().lock(true, pos, count);
    }

    void unlock() { base_t::get_allocator().unlock(); }

    void reallocate(size_type size)
    {
        // TODO: Figure out what to do if reallocation fails here
        base_t::get_allocator().reallocate(true, size);
        m_size = size;
    }

    bool get_handle() const { return true; }
};



// non-singular without implicit size knowledge (standard allocator model)
template <class TAllocator, bool is_stateless>
class handle_descriptor_base<TAllocator, is_stateless, false, false> :
        impl::allocator_descriptor_base<TAllocator, is_stateless>
{

};


// singluar *with* implicit size knowledge
template <class TAllocator, bool is_stateless>
class handle_descriptor_base<TAllocator, is_stateless, true, true>
{

};


template <class TAllocator,
          class TTraits = allocator_traits<typename remove_reference<TAllocator>::type>>
class handle_descriptor :
        public handle_descriptor_base<
            TAllocator,
            TAllocator::is_stateful(),
            false,
            TAllocator::is_singular() >
{

};


}}
