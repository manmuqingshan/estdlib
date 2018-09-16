#include <catch.hpp>

#include <estd/allocators/fixed.h>
#include "estd/memory.h"
#include "estd/exp/tmr.h"
#include "mem.h"

#ifdef UNUSED
// https://stackoverflow.com/questions/621616/c-what-is-the-size-of-an-object-of-an-empty-class
// non-standard, but seemingly reliable way to ensure 0-length empty
// struct
struct empty
{
    char NO_DATA[0];
};
#endif

using namespace estd;

TEST_CASE("memory.h tests")
{
    SECTION("A")
    {
#ifdef UNUSED
        estd::experimental::unique_handle<int, _allocator> uh;
        //auto s = sizeof(empty);
#endif
    }
    SECTION("rebind")
    {
        estd::experimental_std_allocator<int> a;

        float* f = a.rebind_experimental<float>().allocate(10);

        a.rebind_experimental<float>().deallocate(f, 10);
    }
    SECTION("scoped_allocator_adaptor")
    {
        SECTION("basic")
        {
            experimental::scoped_allocator_adaptor<estd::layer1::allocator<int, 512> > a;

            a.allocate(15);
        }
        SECTION("nested")
        {
            experimental::scoped_allocator_adaptor<
                    estd::layer1::allocator<int, 512>,
                    estd::layer1::allocator<int, 256>> a;

            a.allocate(15);
            a.inner_allocator().allocate(20);

            typedef decltype(a) a_type;

            a_type::inner_allocator_type::outer_allocator_type test;

            REQUIRE(test.max_size() == 256);
            test.allocate(15);
        }
    }
    SECTION("experimental")
    {
        SECTION("shared_ptr")
        {
            int val;
            experimental::shared_ptr<int> sp(&val);
        }
        SECTION("shared_ptr2")
        {
            auto f = [](int*) {};
            int val = 5;

            experimental::shared_ptr2_master<int, decltype(f)> sp(&val, f);
            REQUIRE(sp.value().shared_count == 1);
            experimental::shared_ptr2<int> sp2(sp);
            REQUIRE(sp.value().shared_count == 2);
            experimental::shared_ptr2<int> sp3(sp2);
            REQUIRE(sp.value().shared_count == 3);

            sp.reset();
            REQUIRE(sp.value().shared_count == 2);
            sp2.reset();
            REQUIRE(sp.value().shared_count == 1);
        }
    }
}
