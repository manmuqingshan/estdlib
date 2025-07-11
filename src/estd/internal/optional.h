#pragma once

#include "../new.h"
#include "type_traits.h"
#include "fwd/optional.h"
#include "variant/storage.h"

namespace estd {

namespace internal {

// TODO: Experiment with making this -1
template <class T>
struct optional_default_value<T, typename enable_if<is_signed<T>::value>::type> :
    integral_constant<T, 0>
{

};

template <>
struct optional_default_value<bool> :
    integral_constant<bool, false> {};


}

namespace internal {

enum class value_type_morphs
{
    none,
    no_reference
};

// Once in a while we do need to "ditch the ref", such as with evaporators or
// bitwise optional.  This might be a convenient mechanism to help with hat
template <class, value_type_morphs>
struct value_type_morph;

template <class T>
struct value_type_morph<T, value_type_morphs::none>
{
    ESTD_CPP_STD_VALUE_TYPE(T);

#if __cpp_rvalue_references
    typedef T&& rvalue_reference;
    typedef const T&& const_rvalue_reference;
#endif
};

template <class T>
struct value_type_morph<T, value_type_morphs::no_reference>
{
    typedef T value_type;
    typedef T reference;
    typedef const T const_reference;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T rvalue_reference;
    typedef const T const_rvalue_reference;
};


// TODO: Not yet used yet, used to signal that particular type T is a layer1
// creature as well as bit size
template <class T>
struct optional_traits {};

struct optional_tag_base
{
    typedef void optional_tag;

    // DEBT: Are we really using this anymore?  This experiment wasn't a full success
    ESTD_FN_HAS_TYPEDEF_EXP(optional_tag)
};

class optional_has_value
{
    bool initialized_;

protected:
    optional_has_value(bool initialized = false) :
        initialized_(initialized) {}

    void reset() { initialized_ = false; }

    void has_value(bool initialized) { initialized_ = initialized; }

public:
    ESTD_CPP_CONSTEXPR_RET bool has_value() const { return initialized_; }
};


template <class T>
struct optional_value_provider
{
    typedef optional_value_provider this_type;

    ESTD_CPP_STD_VALUE_TYPE(T)

    typedef reference return_type;
    typedef const_reference const_return_type;

private:
    // DEBT: monostate at '1' so that we can feed that into
    // assign_or_init as value to move away from
    variant_storage<T, monostate> storage;

protected:
    ESTD_CPP_DEFAULT_CTOR(optional_value_provider)

    static CONSTEXPR bool real_reference = true;

#if __cpp_variadic_templates
    template <class ...TArgs>
    constexpr explicit optional_value_provider(in_place_t, TArgs&&...args) :
        storage(in_place_index_t<0>{}, std::forward<TArgs>(args)...)
    {

    }

    template <class ...TArgs>
    constexpr explicit optional_value_provider(in_place_conditional_t<0>, bool condition, TArgs&&...args) :
        storage(in_place_conditional_t<0>{}, condition, std::forward<TArgs>(args)...)
    {

    }
#else
    template <class T1>
    optional_value_provider(in_place_conditional_t<0>, bool condition, const T1& v1) :
        storage(in_place_conditional_t<0>(), condition, v1)
    {}

    template <class T1>
    optional_value_provider(in_place_t, const T1& v1) :
        storage(in_place_index_t<0>(), v1)
    {}
#endif

#if __cpp_rvalue_references
    template <class ...TArgs>
    reference emplace(TArgs&&...args)
    {
        return * storage.template emplace<0>(std::forward<TArgs>(args)...);
    }
#else
    template <class T1>
    reference emplace(T1& v)
    {
        return *storage.template emplace<0>(v);
    }
#endif

    void destroy()
    {
        storage.template destroy<0>();
    }

    // NOTE: Direct assignment, doesn't account for dtor or
    // existing object.  Only use this when T is trivial
    void value(const_reference v)
    {
        get<0>(storage) = v;
    }

#if __cpp_rvalue_references
    template <class U>
    void assign_value(bool has_value, U&& u)
    {
        storage.template assign_or_init<0, 1>(has_value, std::forward<U>(u));
    }
#endif

    template <class U>
    void assign_value(bool has_value, const U& u)
    {
        storage.template assign_or_init<0, 1>(has_value, u);
    }

public:
    pointer operator->() { return storage.template get<0>(); }
    const_pointer operator->() const { return storage.template get<0>(); }

    reference value() { return get<0>(storage); }
    ESTD_CPP_CONSTEXPR_RET const_reference value() const
    { return get<0>(storage); }
};


template <class T>
struct optional_base : optional_value_provider<T>,
   optional_has_value
{
    typedef optional_value_provider<T> base_type;

    ESTD_CPP_DEFAULT_CTOR(optional_base)

#if __cpp_variadic_templates
    template <class ...TArgs>
    explicit optional_base(in_place_t, TArgs&&...args) :
        base_type(in_place_t{}, std::forward<TArgs>(args)...),
        optional_has_value(true)
    {}

#else
    optional_base(in_place_t, const T& copy_from) :
        base_type(in_place_t(), copy_from),
        optional_has_value(true)
    {}
#endif

    template <class T2, class TBase>
#if __cpp_constexpr
    constexpr explicit
#endif
    optional_base(const optional<T2, TBase>& copy_from) :
        base_type(in_place_conditional_t<0>(),
            copy_from.has_value(),
            *copy_from),
        optional_has_value(copy_from.has_value())
    {}

#if __cpp_rvalue_references
    template <class T2, class TBase>
    constexpr explicit optional_base(optional<T2, TBase>&& move_from) :
        base_type(in_place_conditional_t<0>{},
            move_from.has_value(),
            // DEBT: Don't even do rvalue/move ctor/assignment if no_value
            std::move(*move_from)),
        optional_has_value(move_from.has_value())
    {}
#endif

    void destroy()
    {
        if(optional_has_value::has_value())
            base_type::destroy();
    }

    ~optional_base()
    {
        destroy();
    }

    void reset()
    {
        destroy();

        optional_has_value::reset();
    }
};

namespace layer1 {

// NOTE: for layer1, T is expected to be trivial
// DEBT: Make a c++20 concept to enforce this

template <class T, unsigned bitsize, T null_value_ = T()>
class optional_base;

// regular null_value flavor
template <class T, T null_value_>
class optional_base<T, 0, null_value_> : public optional_value_provider<T>
{
    typedef optional_value_provider<T> base_type;

protected:
//public:

#if __cpp_rvalue_references
    constexpr explicit optional_base(in_place_t, T&& value) :
        base_type(in_place_t{}, std::forward<T>(value))
    {}
#endif

    // DEBT: #ifdef this out for scenarios only when rvalue is not available
    ESTD_CPP_CONSTEXPR_RET optional_base(in_place_t, const T& value) :
        base_type(in_place_t(), value) {}

    // should always bool == true here
    //optional_base(bool) {}

    ESTD_CPP_CONSTEXPR_RET optional_base() : base_type(in_place_t(), null_value_) {}

    ESTD_CPP_CONSTEXPR_RET optional_base(const optional_base& copy_from) :
        base_type(in_place_t(), copy_from.value())
    {}

    template <class T2, class TBase>
    ESTD_CPP_CONSTEXPR_RET EXPLICIT optional_base(const optional<T2, TBase>& copy_from) :
        base_type(in_place_t(),
            copy_from.has_value() ? copy_from.value() : null_value_)
    {}

public:
    typedef T value_type;

    ESTD_CPP_CONSTEXPR_RET bool has_value() const { return base_type::value() != null_value_; }
#if __cpp_constexpr
    static constexpr bool has_value(bool) { return{}; }
#else
    static inline void has_value(bool) {}
#endif
    void reset() { base_type::value(null_value_); }

    static constexpr value_type null_value = null_value_;
};


// bitwise flavor
template <class T, unsigned bitsize_, T>
class optional_base
{
    struct
    {
        T value_ : bitsize_;
        bool has_value_ : 1;
    };

protected:
    void value(T v) { value_ = v; }
    void has_value(bool initialized) { has_value_ = initialized; }

    static CONSTEXPR bool real_reference = false;

#if __cpp_rvalue_references
    template <class U>
    void assign_value(bool, U&& u)
    {
        value_ = std::forward<U>(u);
    }
#endif

    template <class U>
    void assign_value(bool, const U& u)
    {
        value_ = u;
    }

    // DEBT: Make a converting constructor also
    ESTD_CPP_CONSTEXPR_RET optional_base(const optional_base& copy_from) :
        value_(copy_from.value_),
        has_value_(copy_from.has_value_)
    {}

    ESTD_CPP_CONSTEXPR_RET optional_base() :
        has_value_(false)
    {}

    ESTD_CPP_CONSTEXPR_RET optional_base(in_place_t, const T& v) :
        value_(v), has_value_(true)
    {}

public:
    typedef T value_type;
    typedef value_type return_type;
    typedef value_type const_return_type;

    ESTD_CPP_CONSTEXPR_RET bool has_value() const { return has_value_; }
    void reset() { has_value_ = false; }

    // Deviates from spec here, since bitfield precludes returning a reference
    ESTD_CPP_CONSTEXPR_RET value_type value() const { return value_; }

    static ESTD_CPP_CONSTEXPR_RET unsigned bitsize() { return bitsize_; }
};

#if __cpp_alias_templates
template <class T, T null_value_>
using optional = optional_base<T, is_same<T, bool>::value ? 1 : 0, null_value_>;
#endif

}

}}
