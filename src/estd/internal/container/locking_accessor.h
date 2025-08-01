#pragma once

#include "../type_traits.h"

// Kind of a 2nd attempt at estd::internal::accessor

namespace estd { namespace internal {

template <class Impl>
struct locking_accessor
{
    typedef Impl impl_type;
    typedef locking_accessor this_type;

    ESTD_CPP_STD_VALUE_TYPE(typename impl_type::value_type)

    // This is type for moving pointer/handle around (for iterator use)
    typedef typename impl_type::offset_type offset_type;
    typedef typename impl_type::const_offset_type const_offset_type;

    // Usually this is 'reference' and 'const_reference' respectively, but certain scenarios
    // (such as AVR PGM) these will be 'value_type'
    typedef typename impl_type::locked_type locked_type;
    typedef typename impl_type::const_locked_type const_locked_type;

private:
    impl_type impl_;

public:
    // DEBT: Should be something like 'handle_type', not 'reference'
    //ESTD_CPP_CONSTEXPR_RET EXPLICIT locking_accessor(reference r) : impl_(r) {}
    ESTD_CPP_FORWARDING_CTOR_MEMBER(locking_accessor, impl_)

    ESTD_CPP_CONSTEXPR(17) offset_type h_exp() { return impl_.offset(); }
    constexpr const_offset_type h_exp() const { return impl_.offset(); }

    ESTD_CPP_CONSTEXPR(14) locked_type lock() { return impl_.lock(); }
    constexpr const_locked_type clock() const { return impl_.lock(); }
    ESTD_CPP_CONSTEXPR(14) void unlock() { return impl_.unlock(); }
    ESTD_CPP_CONSTEXPR(14) void cunlock() const { return impl_.unlock(); }

    // Remember, we are an accessor not an iterator, so 'reference' is appropriate
    // here rather than 'pointer'
    // DEBT: Needs filtering
    // DEBT: Leaves unlocked!
    ESTD_CPP_CONSTEXPR(14) reference operator->() { return lock(); }
    ESTD_CPP_CONSTEXPR(14) const_reference operator->() const { return clock(); }

    // DEBT: Leaves unlocked!
    operator locked_type() { return lock(); }
    explicit operator const_locked_type() const { return lock(); }

    this_type& operator=(const value_type& copy_from)
    {
        lock() = copy_from;
        unlock();
        return *this;
    }

    ESTD_CPP_CONSTEXPR(14) bool operator==(const_reference& compare_to) const
    {
        bool r = clock() == compare_to;
        cunlock();
        return r;
    }

    ESTD_CPP_CONSTEXPR(14) bool operator!=(const_reference& compare_to) const
    {
        bool r = clock() != compare_to;
        cunlock();
        return r;
    }

    // DEBT: This needs to be a trait somewhere
    // experimental flag reflecting that this memory block will never move
    static CONSTEXPR bool is_pinned = true;
};

}}
