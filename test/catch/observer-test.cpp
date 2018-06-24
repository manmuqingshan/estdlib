#include <estd/exp/observer.h>
#include <tuple>

#include <catch.hpp>

using namespace estd::experimental;
using namespace estd::experimental::internal;

static int expected;

struct event_1
{
    int data;
};


struct event_2
{
    int data;
};


struct event_3
{
    int data;
};

class StatelessObserver
{
public:
    static void on_notify(int val)
    {
        REQUIRE(val == expected);
    }
};

class FakeBase {};

class StatefulObserver : public FakeBase
{
public:
    int id;

    void on_notify(int val)
    {
        REQUIRE(val == expected);
    }


    void on_notify(event_1 e)
    {
        REQUIRE(e.data == expected);
    }

    void on_notify(event_2 e)
    {
        REQUIRE(e.data == expected);
    }
};


struct OtherStatefulObserver
{
    void on_notify(int val)
    {
        REQUIRE(val == expected);
    }
};

StatefulObserver stateful_observer_1, stateful_observer_2;
OtherStatefulObserver stateful_observer_0;

// clang doesn't like constexpr tuple, and to be fair, c++11 doesn't indicate you can make one in
// the spec (gnu allows it)
#if defined(__GNUC__) && !defined(__clang__)
constexpr auto z = std::make_tuple(1, 2);
#endif


TEST_CASE("observer tests")
{
    stateful_observer_1.id = 1;
    stateful_observer_2.id = 2;

    expected = 3;

    SECTION("stateless")
    {
        stateless_subject<StatelessObserver> ss;

        ss.notify(3);
    }
    SECTION("layer0")
    {
        // FIX: Very unideal because really this still demands a virtual observer, even though it's all wrapped
        // up with template calls
        typedef layer0::subject<StatefulObserver,
                stateful_observer_1,
                stateful_observer_2> s;

        s::notify(3);

        layer0::test_notify(3, stateful_observer_1, stateful_observer_2);

        // probably the best we're gonna do is something like a tuple, which still has a potential memory footprint
        // but for layer0-ish/constant scenarios perhaps more practical than a layer1::vector

        expected = 5;

        /*
        s::notify(3); */
        SECTION("subject2")
        {
            auto s = layer0::make_subject(
                    StatefulObserver(),
                    stateful_observer_0,
                    stateful_observer_1,
                    stateful_observer_2);

            int sz = sizeof(s);

            s.notify(5);
        }
        SECTION("constexpr subject2")
        {
            constexpr auto s = layer0::make_subject_const(stateful_observer_0, stateful_observer_1, stateful_observer_2);

            // still 24; guess this makes sense, can't constexpr-optimize away instance variables
            int sz = sizeof(s);

            //s.notify(5);
            REQUIRE(sz > 0);
        }
        SECTION("event overloading")
        {
            auto s = layer0::make_subject(stateful_observer_1, stateful_observer_2);

            s.notify(event_1 { 5 });

            expected = 3;

            s.notify(event_2 { 3 });
        }
        SECTION("void event")
        {
            void_subject s;

            // resolves to noop, just in here to make sure it compiles really
            s.notify(event_1 { 5 });
        }
    }
}
