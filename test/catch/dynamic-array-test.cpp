#include <estd/internal/dynamic_array.h>

#include <catch.hpp>

using namespace estd;
using namespace estd::internal;

struct pa_impl
{
    //ESTD_CPP_STD_VALUE_TYPE(int)

    //int data_[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
};

template <>
struct experimental::private_array<pa_impl> : public array<int, 10>
{
    using base_type = array<int, 10>;

    private_array() : base_type{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }
    {

    }

    void copy_ll(int* dest, int count) const
    {
        estd::copy_n(base_type::data(), count, dest);
    }
};

TEST_CASE("dynamic array")
{
    using da1_type = internal::dynamic_array<internal::impl::dynamic_array<
        estd::internal::single_fixedbuf_allocator<int, 20>, void> >;
    using pa1_type = experimental::private_array<pa_impl>;

    da1_type da1;
    pa1_type pa1;

    SECTION("basics")
    {
        da1.push_back(5);

        REQUIRE(da1.size() == 1);

        da1.append(pa1);

        REQUIRE(da1.size() == 11);

        REQUIRE(da1[2] == 1);
    }
    SECTION("iterate")
    {
        da1_type::accessor a(da1.get_allocator(), 0);
        da1.append(pa1);

        auto it = da1.begin();

        const auto& v = *it;

        REQUIRE(v == 0);
    }
    SECTION("overflow")
    {
#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
        // DEBT: Consider adding assignment operator to append_result
        auto r = da1.append(pa1);
        REQUIRE(r.has_value());
        auto r2 = da1.append(pa1);
        REQUIRE(r2.has_value());
        //auto r3 = da1.append(pa1);
        //REQUIRE(r3.has_value() == false);
        //REQUIRE(r3.error() == 0);
#endif
    }
}
