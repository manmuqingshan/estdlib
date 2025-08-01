#pragma once

#include "../../type_traits.h"

#include "fwd.h"

#if defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)

// Arduino compatibility
#pragma push_macro("F")
#undef F

namespace estd { namespace detail { namespace impl {


template <typename Result, typename... Args, fn_options o>
struct function_fnptr2<Result(Args...), o>
{
#if FEATURE_ESTD_GH135
    enum modes
    {
        COPY,
        MOVE,
        DELETE
    };
#endif

    static constexpr fn_options options = o;

    struct utility_base
    {
        // DEBT: Make this optional and default to off

        // mode, model in, model out
        typedef void (*utility_type)(modes, void*, void*);
        utility_type u_{};
    };

    // this is a slightly less fancy more brute force approach to try to diagnose esp32
    // woes
    struct model_base : conditional_t<false,
        monostate,
        utility_base>
    {
        typedef Result (*function_type)(void*, Args...);
        typedef void (*deleter_type)();

        const function_type _f;
#if GITHUB_ISSUE_39_EXP
        const deleter_type _d;
#endif

        constexpr explicit model_base(
            function_type f,
            deleter_type d = nullptr) :
            _f(f)
#if GITHUB_ISSUE_39_EXP
            ,
            _d{d}
#endif
        {}

        model_base(const model_base& copy_from) = default;
        // DEBT: For some reason ESP32's default move constructor
        // doesn't initialize _f
        //concept_fnptr2(concept_fnptr2&& move_from) = default;
        constexpr model_base(model_base&& move_from) noexcept:
            _f(std::move(move_from._f))
#if GITHUB_ISSUE_39_EXP
            ,
            _d{std::move(move_from._d)}
#endif
        {}

        inline Result _exec(Args&&...args)
        {
            return _f(this, std::forward<Args>(args)...);
        }

        inline Result operator()(Args&&...args)
        {
            return _f(this, std::forward<Args>(args)...);
        }

#if FEATURE_ESTD_GH135
        void copy_to(model_base* dest)
        {
            utility_base::u_(COPY, this, dest);
        }

        void move_to(model_base* dest)
        {
            utility_base::u_(MOVE, this, dest);
        }
#endif
    };

    template <typename F>
    struct model : model_base
    {
        using base_type = model_base;

#if FEATURE_ESTD_GH135
        static void utility(modes mode, void* src, void* dest)
        {
            auto this_ = static_cast<model*>(src);

            switch(mode)
            {
                case COPY:
                    //new (dest) model(this_->f);
                    break;

                case MOVE:
                    new (dest) model(std::move(this_->f));
                    break;

                case DELETE:
                    break;
            }
        }
#endif

        //template <typename U>
        constexpr explicit model(F&& u) :
            base_type(static_cast<typename base_type::function_type>(&model::exec)),
            f(std::forward<F>(u))
        {
#if FEATURE_ESTD_GH135
            base_type::u_ = utility;
#endif
        }

        /*
        model_fnptr2(const model_fnptr2& copy_from) = default;
        //model_fnptr2(model_fnptr2&& move_from) = default;
        model_fnptr2(model_fnptr2&& move_from) :
            base_type(std::move(move_from)),
            f(std::move(move_from.f))
        {} */

        F f;

        /*
        static void dtor(void* _this)
        {
            ((model*)_this)->f.~F();
        }   */

        // TODO: Consolidate different models down to a model_base since they
        // all need this exec function
        Result operator()(Args&&...args)
        {
            return f(std::forward<Args>(args)...);
        }

        static Result exec(void* _this, Args...args)
        {
            F& f = ((model*)_this)->f;

            return f(std::forward<Args>(args)...);
        }
    };


    // 13MAY24 MB EXPERIMENTAL replacement for 'thisify'
    template <class T, Result (T::*f)(Args...)>
    struct method_model : model_base
    {
        constexpr explicit method_model(T* t) :
            model_base(static_cast<typename model_base::function_type>(&method_model::exec)),
            object_{t}
        {}

        T* const object_;

        constexpr Result operator()(Args&&...args) const
        {
            return (object_->*f)(std::forward<Args>(args)...);
        }

        // DEBT: Use rvalue here
        static Result exec(void* this_, Args...args)
        {
            return (*((method_model*)this_))(std::forward<Args>(args)...);
        }
    };
};

// Special version which calls dtor right after function invocation
template <typename Result, typename... Args>
struct function_fnptr2_opt<Result(Args...)>
{
    struct model_base
    {
        typedef Result (*function_type)(void*, Args...);

        const function_type f;

        constexpr explicit model_base(function_type f) : f(f) {}

        inline Result operator()(Args&&...args)
        {
            return f(this, std::forward<Args>(args)...);
        }
    };


    template <typename F>
    struct model_void : model_base
    {
        using base_type = model_base;
        using typename base_type::function_type;

        //template <typename U>
        constexpr explicit model_void(F&& u) :
            base_type(
                static_cast<function_type>(&model_void::exec)),
            f(std::forward<F>(u))
        {
        }

        F f;

        static void exec(void* this_, Args...args)
        {
            F& f = ((model_void*)this_)->f;
            f(std::forward<Args>(args)...);
            f.~F();
        }
    };

    template <typename F>
    struct model_nonvoid : model_base
    {
        using base_type = model_base;
        using typename base_type::function_type;

        //template <typename U>
        constexpr explicit model_nonvoid(F&& u) :
            base_type(
                static_cast<function_type>(&model_nonvoid::exec)),
            f(std::forward<F>(u))
        {
        }

        F f;

        static Result exec(void* this_, Args...args)
        {
            F& f = ((model_nonvoid*)this_)->f;
            Result r = f(std::forward<Args>(args)...);
            f.~F();
            return r;
        }
    };

    template <class F>
    using model = conditional_t<is_void<Result>::value,
        model_void<F>,
        model_nonvoid<F> >;
};

}}}

#pragma pop_macro("F")

#endif
