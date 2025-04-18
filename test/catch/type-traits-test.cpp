#include <catch2/catch_all.hpp>

#include <estd/type_traits.h>
#include <estd/limits.h>
#include <estd/string.h>
#include <estd/internal/raw/variant.h>

#include "test-data.h"

using namespace estd;

#include "macro/push.h"

static test::Dummy dummy(7, "hi2u");

// EXPERIMENTAL, but looking good
template <class T, T* v>
struct integral_pointer
{
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef integral_pointer type;

    static constexpr pointer value = v;
    static constexpr T& ref = *v;

    constexpr pointer operator->() const
    {
        return value;
    }

    constexpr reference operator*() const { return ref; }
};


#define ESTD_INTEGRAL_POINTER(v)    integral_pointer<decltype(v), &v>

TEST_CASE("type traits tests")
{
    SECTION("integral_constant")
    {
        using type1 = integral_constant<int, 5>;
        using type2 = integral_constant<test::Dummy*, &dummy>;
        using type3 = integral_pointer<test::Dummy, &dummy>;
        using type4 = ESTD_INTEGRAL_POINTER(dummy);

        REQUIRE(type1() == 5);
        REQUIRE(type2()()->val1 == 7);
        REQUIRE(type3::ref == dummy);

        type3::ref.val1++;

        REQUIRE(dummy.val1 == 8);
        REQUIRE(type3()->val1 == 8);

        test::Dummy& d = *type3();
        REQUIRE(d.val1 == 8);
        (*type3{}).val1 = 5;
    }
    SECTION("make_signed")
    {
        typedef make_signed<uint16_t>::type should_be_int16_t;

        REQUIRE(sizeof(should_be_int16_t) == sizeof(int16_t));
        REQUIRE(is_signed<should_be_int16_t>::value);
        REQUIRE(!is_signed<uint16_t>::value);
    }
    SECTION("make_unsigned")
    {
        using should_be_uint16_t = make_unsigned_t<int16_t>;

        REQUIRE(is_unsigned<should_be_uint16_t>::value);
        REQUIRE(is_same<uint16_t, should_be_uint16_t>::value);
    }
    SECTION("decay")
    {
        int digits = numeric_limits<decay_t<uint8_t> >::digits;
        REQUIRE(digits == 8);
        digits = numeric_limits<decay_t<int16_t> >::digits;
        REQUIRE(digits == 15);
        digits = numeric_limits<decay_t<uint16_t> >::digits;
        REQUIRE(digits == 16);
    }
    SECTION("remove_reference")
    {
        constexpr int value = 5;
        int _digit = value;
        int& digit = _digit;
        // removing reference means we are back to an int vs int&
        estd::remove_reference_t<decltype(digit)> digit_copy = digit;

        REQUIRE(digit_copy == _digit);

        digit_copy = value * 2;

        REQUIRE(_digit == value);
        REQUIRE(digit == value);
        REQUIRE(digit_copy == value * 2);
    }
    SECTION("char_traits")
    {
        SECTION("char")
        {
            REQUIRE(is_signed<char_traits<char>::off_type>::value);
            REQUIRE(!is_signed<char_traits<char>::pos_type>::value);
        }
        SECTION("const char")
        {
            REQUIRE(is_signed<char_traits<const char>::off_type>::value);
            REQUIRE(!is_signed<char_traits<const char>::pos_type>::value);
        }
        SECTION("uint8_t")
        {
            REQUIRE(is_signed<char_traits<uint8_t>::off_type>::value);
            REQUIRE(!is_signed<char_traits<uint8_t>::pos_type>::value);
        }
    }
    SECTION("is_base_of")
    {
        struct Parent {};

        SECTION("single inheritance")
        {
            struct Child : Parent {};

            REQUIRE(is_base_of<Parent, Child>::value);
        }
        SECTION("multiple inheritance")
        {
            struct Parent2 {};
            struct Parent3 {};

            struct Child : Parent, Parent2, Parent3 {};

            REQUIRE(is_base_of<Parent, Child>::value);
            REQUIRE(is_base_of<Parent2, Child>::value);
            REQUIRE(is_base_of<Parent3, Child>::value);
        }
    }
    SECTION("underlying_type")
    {
        enum test1 : int16_t { a, b, c };

        constexpr bool v = is_same<underlying_type<test1>::type, int16_t>::value;

        REQUIRE(v == true);
    }
    SECTION("is_convertible")
    {
        REQUIRE(is_convertible<int, long>::value == true);
        REQUIRE(is_convertible<test::ChildOfDummy, test::Dummy>::value == true);
        REQUIRE(is_convertible<test::Dummy, test::ChildOfDummy>::value == false);
        REQUIRE(is_convertible<test::ChildOfDummy*, test::Dummy*>::value == true);
        REQUIRE(is_convertible<test::Dummy*, test::ChildOfDummy*>::value == false);

        bool v1 = is_convertible<monostate, float>::value;

        REQUIRE(v1 == false);

        v1 = is_convertible<float, monostate>::value;

        REQUIRE(v1 == false);

        v1 = is_convertible<int, const char*>::value;

        REQUIRE(v1 == false);

        v1 = is_constructible<const char*, int>::value;

        REQUIRE(v1 == false);

        v1 = is_constructible<int, const char*>::value;

        REQUIRE(v1 == false);

        v1 = is_convertible<double, char>::value;

        REQUIRE(v1 == true);

        v1 = std::is_convertible<double, char>::value;

        REQUIRE(v1 == true);
    }
    SECTION("is_assignable")
    {
        bool v1 = is_assignable<const char*&, char*>::value;

        REQUIRE(v1);

        v1 = is_assignable<test::NonTrivial&, test::NonTrivial>::value;

        REQUIRE(v1 == false);
    }
    SECTION("is_constructible")
    {
        bool v1 = is_nothrow_constructible<test::Dummy, test::Dummy>::value;

        REQUIRE(v1);

        v1 = is_nothrow_constructible<test::Dummy, test::Dummy&&>::value;

        REQUIRE(v1 == true);

        v1 = is_nothrow_constructible<test::Dummy, const test::Dummy&>::value;

        REQUIRE(v1 == false);

        v1 = is_nothrow_move_constructible<test::Dummy>::value;

        REQUIRE(v1);
    }
    SECTION("is_floating_point")
    {
        bool v1 = is_floating_point<double>::value;

        REQUIRE(v1);
    }
    SECTION("non-aliased")
    {
        // Using non-aliased flavors here, just to ensure proper implementation (rather than ONLY
        // test ON AVRs)

        SECTION("is_constructible")
        {
            bool v = detail::is_constructible<int, const char*>::value;

            REQUIRE(v == false);

            v = detail::is_constructible<const char*, int>::value;

            REQUIRE(v == false);
        }
    }
}

#include "macro/pop.h"
