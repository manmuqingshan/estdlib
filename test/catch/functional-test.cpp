#include <catch2/catch_all.hpp>

#include <estd/functional.h>

#include "test-data.h"

#include "macro/push.h"

static const char* got_something = NULLPTR;

int do_something(const char* msg)
{
    got_something = msg;
    //printf("I did something: %s\n", msg);

    return -1;
}

template <typename F, class ...Args>
int do_something_inspector(F&& f, Args&&...args)
{
    using traits = estd::function_traits<F>;

    static_assert(estd::is_same<typename traits::return_type, int>::value, "functor must return int");

    return f(std::forward<Args>(args)...);
}

using namespace estd;

// temporarily switching this on or off here as I build out experimental pre C++03 tuple
#ifdef FEATURE_CPP_VARIADIC


void forwarder_func(int val, estd::detail::function<void(int)> f)
{
    f(val);
}

template <class ...TArgs>
void forwarder(TArgs&&...args)
{
    forwarder_func(std::forward<TArgs>(args)...);
}

template <typename T>
using fb = detail::v2::function<T, detail::impl::function_virtual>;

template <template <class, detail::impl::fn_options> class Impl, typename F>
struct ProvidedTest1;

template <template <class, detail::impl::fn_options> class Impl, typename TResult, typename ...TArgs>
struct ProvidedTest1<Impl, TResult(TArgs...)>
{
    typedef TResult (test)(TArgs...);

    static TResult test2(TArgs...) { return TResult(); }
};

template <template <class, detail::impl::fn_options> class Impl, typename F, typename TDummy>
struct ProvidedTest2;

template <template <class, detail::impl::fn_options> class Impl, typename TResult, typename ...TArgs, class TDummy>
struct ProvidedTest2<Impl, TResult(TArgs...), TDummy>
{
    TDummy value2;

    typedef TResult (test)(TArgs...);

    static TResult test2(TArgs...) { return TResult(); }
};

struct TestFunctor1
{
    int value = 0;

    int operator()(int v)
    {
        value += v;
        return value;
    }
};

// Temporary helper to disable errors while we bring up issue39
#define ISSUE_39_BRINGUP 1

#if ISSUE_39_BRINGUP
struct TestFunctorProvider
{
    using this_type = TestFunctorProvider;
    using function_type = estd::detail::function<int(int)>;

    int value = 0;

    struct base
    {
        //this_type* const this_;
    };

    struct adder //: base
    {
        this_type* const this_;
        int operator()(int v) const
        {
            return this_->value += v;
        }
    };

    struct subtractor //: base
    {
        this_type* const this_;
        int operator()(int v) const
        {
            return this_->value -= v;
        }
    };

    function_type::model<adder> adder_model;
    function_type::model<subtractor> subtractor_model;

    function_type which;

    TestFunctorProvider() :
        adder_model{adder{this}},
        subtractor_model{subtractor{this}},
        which{&adder_model}
    {}
};
#endif

TEST_CASE("functional")
{
    SECTION("function")
    {
        SECTION("fn_exp")
        {
            using fn_type = detail::v2::function<void(), detail::impl::function_fnptr2_opt>;
            int counter = 0;
            test::Dummy dummy;
            dummy.inc_on_destruct = &counter;

            auto m = fn_type::make_model([&, dummy]{ ++counter; });

            fn_type f(&m);

            f();

            REQUIRE(counter == 2);
        }
        SECTION("experimental")
        {
            SECTION("simplest lambda")
            {
                estd::experimental::function<int()> f = []() { return 5; };

                int val = f();

                REQUIRE(val == 5);
            }
            SECTION("ref capture lambda")
            {
                int val2 = 5;
                int val3 = 0;

                estd::experimental::function<int(int)> f = [&val2](int x) { return val2++; };

                REQUIRE(f(0) == 5);
                f = [&](int x) { ++val3; return val2++; };
                REQUIRE(f(1) == 6);
                REQUIRE(val2 == 7);
                REQUIRE(val3 == 1);
            }
            SECTION("in place execution")
            {
                int val = 0;

                estd::experimental::function<void(int)>([&](int v) { val = v; })(5);

                REQUIRE(val == 5);
            }
            SECTION("passing to another")
            {
                int val = 0;

                estd::experimental::function<void(int)> f([&](int v) { val = v; });

                SECTION("function_base")
                {
                    auto tester = [](estd::detail::function<void(int)> _f, int param)
                    {
                        _f(std::move(param));
                    };

                    tester(f, 5);

                    REQUIRE(val == 5);
                }
                SECTION("function_base perfect forwarding")
                {
                    forwarder(5, f);

                    REQUIRE(val == 5);
                }
            }
            SECTION("complex parameter lambda")
            {
                int val2 = 5;
                int val3 = 0;

                estd::experimental::function<int(int*, int)> f = [&val2](int* dest, int x) { return ++val2 + x; };

                REQUIRE(f(&val3, 1) == 7);

                int _dest = 0;

                auto f2 = estd::experimental::function<void(int*, int)>::make_inline2(
                    [&](int* dest, int x)
                {
                    *dest += x;
                });

                f2(&_dest, 1);
                REQUIRE(_dest == 1);
            }
            SECTION("make_inline (deprecated)")
            {
                auto i = estd::experimental::function<int(int)>::make_model([](int x) { return x + 1; });

                REQUIRE(i.operator()(1) == 2);

                estd::detail::function<int(int)> f(&i);

                REQUIRE(f.operator()(1) == 2);

                int outside_scope_value = 5;
                auto l = [&](int x) { return x + outside_scope_value; };

                estd::experimental::inline_function<decltype(l), int(int)> _if(std::move(l));
                /*
                auto _if2 =
                    estd::experimental::make_inline_function<decltype(l), int(int)>(std::move(l)); */

                REQUIRE(_if(5) == 10);

                auto _if3 = estd::experimental::function<int(int)>::make_inline2(
                    [&](int x) { return x * outside_scope_value; });

                REQUIRE(_if3(5) == 25);

                estd::detail::function<int(int)> fb1(_if3);

                REQUIRE(fb1(5) == 25);
            }
            SECTION("upcast")
            {
                auto dynamic_f = new estd::experimental::function<int(int)>([](int v)
                {
                    return v;
                });

                SECTION("initialization")
                {
                    estd::detail::function<int(int)> f(*dynamic_f);
                }
                SECTION("assignment")
                {
                    estd::detail::function<int(int)> f;

                    f = *dynamic_f;
                }

                delete dynamic_f;
            }
            SECTION("context_function")
            {
                ContextTest ctx;

                estd::experimental::context_function<
                    decltype(&ContextTest::add), &ContextTest::add> f(&ctx);

                f(5);

                REQUIRE(ctx.val == 5);
            }
            SECTION("bind")
            {
                SECTION("helper")
                {
                    // hitting issue described here:
                    // https://stackoverflow.com/questions/43560492/how-to-extract-lambdas-return-type-and-variadic-parameters-pack-back-from-gener
                    // I am certain at one time I went down the function_traits/callable_traits route.  Perhaps it's
                    // lingering on an unfinished branch somewhere?
                    //typedef estd::experimental::bind_helper<decltype([](){})> bh;
                }
                SECTION("lambda")
                {
                    int val = 0;
                    //auto b = estd::experimental::bind([&](){ val += 2; });
                }
                SECTION("to member")
                {
                    ContextTest ctx;

                    //estd::experimental::bind(&ContextTest::add, &ctx);
                }
            }
            SECTION("method deduction")
            {
                ContextTest ctx;

                internal::impl::method_model_helper2::do_something(&ctx, &ContextTest::add);
                //internal::impl::method_model_helper<ContextTest, &ContextTest::add>
            }
            SECTION("13MAY24 new method_model")
            {
                ContextTest ctx;

                SECTION("fnptr1")
                {
                    detail::impl::function_fnptr1<int(int)>::
                        method_model<ContextTest, &ContextTest::add> m(&ctx);

                    m(5);

                    REQUIRE(ctx.val == 5);
                }
                SECTION("fnptr2")
                {
                    detail::impl::function_fnptr2<int(int)>::
                        method_model<ContextTest, &ContextTest::add> m(&ctx);

                    m(5);

                    REQUIRE(ctx.val == 5);
                }
                SECTION("virtual")
                {
                    detail::impl::function_virtual<int(int)>::
                        method_model<ContextTest, &ContextTest::add> m(&ctx);

                    m(5);

                    REQUIRE(ctx.val == 5);
                }
            }
        }
    }
    SECTION("obsolete")
    {
        SECTION("basic")
        {
            // Doesn't seem to work at this point
            //estd::obsolete::function<int()> f = []() { return 5; };
        }
        SECTION("bind")
        {
            auto b = estd::obsolete::bind(do_something, "hello");

            int sz = sizeof(b);

            REQUIRE(b() == -1);

            REQUIRE(std::string(got_something) == "hello");
        }
    }
    SECTION("impls")
    {
        SECTION("explicit")
        {
            SECTION("virtual")
            {
                using _fb = detail::v2::function<void(int), detail::impl::function_virtual>;

                struct model : _fb::model_base
                {
                    void operator()(int v) override
                    {
                        counter += v;
                    }

                    int counter = 0;

#if FEATURE_ESTD_GH135
                    void copy_to(model_base* dest) override
                    {

                    }

                    void move_to(model_base* dest) override
                    {

                    }
#endif
                };

                model m;
                _fb f(&m);

                f(5);

                REQUIRE(m.counter == 5);
            }
            SECTION("fnptr1 (default)")
            {
                using _fb = detail::v2::function<void(int), detail::impl::function_fnptr1>;
                int value = 0;

                //auto m = estd::internal::make_model<void(int)>(
                auto m = _fb::make_model(
                    [&](int v) { value += v; });

                _fb f(&m);

                f(5);

                REQUIRE(value == 5);
            }
            SECTION("fnptr2")
            {
                typedef estd::detail::function<void(int),
                    estd::detail::impl::function_fnptr2<void(
                    int)> > _fb;

                SECTION("default model")
                {
                    estd::test::Dummy dummy1;

                    int counter = 0;

                    dummy1.inc_on_destruct = &counter;

                    {
                        auto model = _fb::make_model([dummy1](int)
                        {

                        });

                        _fb f(&model);

                        f(0);

                        REQUIRE(counter == 0);
                    }

                    REQUIRE(counter == 1);
                }
                SECTION("custom model")
                {
                    struct model : _fb::model_base
                    {
                        typedef _fb::model_base base_type;

                        int counter = 0;

                        model() : base_type(&_exec)
                        {}

                        static void _exec(void* _this, int v)
                        {
                            ((model*)_this)->counter += v;
                        }
                    };

                    model m;
                    _fb f(&m);

                    f(5);

                    REQUIRE(m.counter == 5);
                }
            }
            SECTION("fnptr2_opt")
            {
                // ISSUE_39_BRINGUP
                typedef estd::detail::function<int(void),
                    estd::detail::impl::function_fnptr2_opt<int(
                        void)> > _fb;
                typedef estd::detail::function<void(void),
                    estd::detail::impl::function_fnptr2_opt<void(
                        void)> > _fb2;

                int counter = 0;
                int counter2 = 0;

                estd::test::Dummy dummy1, dummy2;

                dummy1.inc_on_destruct = &counter;
                dummy2.inc_on_destruct = &counter2;

                {
                    // Beware there's a copy and a move of 'dummy1'
                    auto model =
                        _fb::make_model([dummy1, &counter]
                        {
                            ++counter;
                            return 0;
                        });

                    auto model2 = _fb2::make_model([dummy2]
                    {

                    });

                    {
                        _fb f(&model);
                        _fb2 f2(&model2);

                        f();
                        f2();

                        REQUIRE(counter == 2);
                        REQUIRE(counter2 == 1);
                    }

                    REQUIRE(counter == 2);
                }

                // NOTE: It's a feature not a bug, dtor is called once too many times.
                // that is because _opt flavor is conceived of to work in a placement new
                // scenario where the "delete" never happens.
                REQUIRE(counter == 3);
                REQUIRE(counter2 == 2);
            }
        }
        SECTION("aliased")
        {
            fb<void(int&&)> f;

            struct model : decltype(f)::model_base
            {
                int counter = 0;

                void operator()(int&& v) override
                {
                    counter += v;
                }
            };

        }
    }
    SECTION("impl")
    {
        using impl_type = estd::detail::impl::function_fnptr1<int(int)>;
        using fn1_type = detail::v2::function<int(int), detail::impl::function_fnptr1>;

        ContextTest context;
        internal::impl::function_context_provider<
            estd::detail::impl::function_fnptr1, int(int)>::model<ContextTest, &ContextTest::add> m(&context);
        fn1_type f(&m);

        f(5);

        REQUIRE(context.val == 5);

        typedef fn1_type::imbue<ProvidedTest1> p1;

        p1::test2(5);

        //p1::test2(5);

        internal::impl::function_context_provider<
            estd::detail::impl::function_fnptr1,
            fn1_type>::model<ContextTest, &ContextTest::add>
            m3(&context);

        //internal::impl::method_model<int(int), ContextTest, &ContextTest::add> m4(&context);
        internal::impl::method_type<int(int), ContextTest> m4;
        internal::impl::method_model<
            int(int), ContextTest, &ContextTest::add,
            estd::detail::impl::function_fnptr1> m5(&context);

        // Doesn't play nice, presumably because TArgs2... doesn't handle the ContextTest::add part well
        //fn1_type::imbue<internal::impl::method_model, ContextTest, &ContextTest::add> m6(&context);

        fn1_type  f_m5(&m5);

        f_m5(2);

        REQUIRE(context.val == 7);

        SECTION("provided")
        {
            // Doing 'model' using this 'provider' technique in hopes of reducing verbosity
            // However, so far it doesn't really seem to be much help
            typedef fn1_type::imbue<internal::impl::function_context_provider>::
            model<ContextTest, &ContextTest::add> p2;

            p2 m2(&context);

            typedef fn1_type::imbue<ProvidedTest2, float> type1;

            type1 p3;

            p3.test2(5);
            REQUIRE(estd::is_same<decltype(type1::value2), float>::value);
        }
        SECTION("functor")
        {
            SECTION("fundamental")
            {
                TestFunctor1 functor;
                fn1_type::model<TestFunctor1> m2(std::forward<TestFunctor1>(functor));
                //auto m2 = fn1_type::make_model(std::forward<TestFunctor1>(functor));
                int v = m2.exec(5);

                REQUIRE(v == 5);

                v = m2.exec(5);

                REQUIRE(v == 10);

                //REQUIRE(functor.value == 10);
            }
#if ISSUE_39_BRINGUP
            SECTION("within a class")
            {
                TestFunctorProvider tfp;

                tfp.which(5);
                tfp.which(2);

                REQUIRE(tfp.value == 7);
            }
#endif
        }
    }
    SECTION("function_traits")
    {
        SECTION("detail::function")
        {
            typedef estd::detail::function<int(int)> fn_type;
            using traits = function_traits<fn_type>;

            REQUIRE(estd::is_same<traits::arg<0>, int>::value);
            REQUIRE(estd::is_same<traits::arg<0>, float>::value == false);

            static_assert(traits::is_method == false, "");
        }
        SECTION("direct")
        {
            typedef function_traits<decltype(do_something)> traits;

            REQUIRE(estd::is_same<traits::arg<0>, const char*>::value);
            REQUIRE(estd::is_same<traits::return_type, int>::value);

            static_assert(traits::is_method == false, "");
        }
        SECTION("functor")
        {
            do_something_inspector(do_something, "hello");
        }
        SECTION("lambda")
        {
            auto l = [](int v) { return v + 5; };

            typedef function_traits<decltype(l)> traits;

            static_assert(estd::is_same<traits::return_type, int>::value, "Should be int");
            static_assert(traits::nargs == 1, "Should be 1 argument only");
            static_assert(traits::is_method, "");

            int v = do_something_inspector(std::forward<decltype(l)>(l), 5);

            REQUIRE(v == 10);

            v = do_something_inspector([&](int i, int j) { return i + j + l(5); }, 4, 5);

            REQUIRE(v == 19);
        }
        SECTION("member function")
        {
            using traits = function_traits<decltype(&test::TestB::add)>;

            static_assert(traits::nargs == 1, "");
            static_assert(estd::is_same<traits::arg<0>, int>::value, "");
            static_assert(traits::is_method, "");
        }
        SECTION("uso")
        {
            //using traits = function_traits<int>;
        }
    }
    SECTION("thisify")
    {
        ContextTest ctx;

        SECTION("int(int)")
        {
            estd::internal::thisify_function<int(int)>::
                model<ContextTest, &ContextTest::add> m2(&ctx);

            estd::internal::impl::method_model<
                int(int),
                ContextTest, &ContextTest::add,
                detail::impl::function_fnptr1>
                m(&ctx);

            //int sz = sizeof(m.f);

            // TODO: Do spread of comparisons based on which impl is used
            //REQUIRE(sizeof(m) == sizeof(ContextTest*) + sizeof(m.f) + sizeof(m.d));

            detail::v2::function<int(int), detail::impl::function_fnptr1>
                f(&m);

            f(5);

            REQUIRE(ctx.val == 5);

            f = &m2;

            REQUIRE(f);

            f(2);

            REQUIRE(ctx.val == 7);

            f = nullptr_t{};

            REQUIRE(!f);
        }
        SECTION("void(void)")
        {
            estd::internal::impl::method_model<
                void(void),
                ContextTest, &ContextTest::add2
                >
                m(&ctx);

            // NOTE: In this case we actually are using 'm' to copy
            // into f and technically m storage is not required
            estd::internal::thisify_function<void(void)> f(m);

            f();

            REQUIRE(ctx.val == 7);
        }
        SECTION("tag/indicator")
        {
            // TODO: Try passing in a tag or an actual function pointer which
            // is immediately tossed out
        }
    }
    SECTION("contextify")
    {
        ContextTest c;
        estd::internal::contextify_function<int(int)>::model<ContextTest*, &ContextTest::add3> m(&c);
        estd::detail::function<int(int), detail::impl::function_fnptr1<int(int)>> f(&m);

        f(5);

        REQUIRE(c.val == 5);
    }
    SECTION("static_function")
    {
        int value = 0;

        // As the name implies, I am not convinced this will play nice when used
        // inside a method - how is one static F&&/model going to differentiate
        // between multiple 'this' pointers?  Then again if the storage is a pointer
        // to the this pointer, that may be different.  We definitely don't expect
        // concurrency/multi thread to behave well here.
        estd::experimental::static_function<int(int)> f([&](int v)
        {
            return value += v;
        });

        f(5);

        REQUIRE(value == 5);
    }
    // DEBT: Revise nomenclature and move to organized location alongside
    // similar unit tests
    SECTION("trivial (movable)")
    {

    }
#if FEATURE_ESTD_GH135
    SECTION("move")
    {
        test::Dummy dummy(0, "hello");
        test::Dummy* dummy_ptr = &dummy;

        SECTION("fnptr2")
        {
            using fb = detail::v2::function<void(int), detail::impl::function_fnptr2>;
            using model_base = typename fb::model_base;

            auto m1 = fb::make_model([=](int v)
            {
                dummy_ptr->val1 += v;
                if(dummy.moved_)    ++dummy_ptr->val1;
            });

            fb fb1(&m1);

            fb1(5);

            REQUIRE(dummy.val1 == 6);

            estd::byte raw2[sizeof(m1)], raw3[sizeof(m1)];

            fb1.move_to((model_base*)raw2);

            fb fb2((model_base*) raw2);

            fb2(5);

            REQUIRE(dummy.val1 == 12);
        }
        SECTION("virtual")
        {
            using fb = detail::v2::function<void(int), detail::impl::function_virtual>;
            using model_base = typename fb::model_base;
            using fv = internal::function_view<void(int), detail::impl::function_virtual>;

            auto m1 = fb::make_model([=](int v)
            {
                dummy_ptr->val1 += v;
                if(dummy.moved_)
                    dummy_ptr->val1 += 1;
            });

            fb fb1(&m1);

            fb1(5);

            REQUIRE(dummy.val1 == 6);

            estd::byte raw2[sizeof(m1)], raw3[sizeof(m1)];

            //fv fv1(&m1, sizeof(m1));

            //fv1.function()(5);

            fb1.move_to((model_base*)raw2);

            fb fb2((model_base*) raw2);

            fb2(5);

            REQUIRE(dummy.val1 == 12);

            fb2.copy_to((model_base*)raw3);

            fb fb3((model_base*) raw3);

            fb3(5);

            REQUIRE(dummy.val1 == 17);
        }
    }
#endif
}

#endif

#include "macro/pop.h"
