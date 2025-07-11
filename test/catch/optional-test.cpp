#include <catch2/catch_all.hpp>

#include <estd/optional.h>

#include "test-data.h"

using namespace estd;

estd::optional<int> returning_optional(int val)
{
    if(val == 10) return nullopt;

    if(val == 20) return {};

    return val;
}

template <typename T, class TBase>
static void suite(estd::optional<T, TBase> o, T compare_to)
{
    REQUIRE(!o);
    bool result = o != T();
    REQUIRE(result);
    result = o == T();
    REQUIRE(!result);
    REQUIRE(!(o > compare_to));
    REQUIRE(!(o < compare_to));

    o = compare_to;

    result = o == compare_to;

    //REQUIRE(val > 4);
    REQUIRE(result);
}

enum SpecializedEnum
{
    Value1,
    Value2
};

enum PlainOldEnum
{
    POE_UNINITIALIZED = 0,
    POE_1,
    POE_2
};

struct BitField
{
    union
    {
        struct
        {
            int val1 : 3;
            int val2 : 10;
        };

        int raw = 0;
    };
};

// Any time SpecializedEnum is used in regular estd::optional, it routes to this base class
// See below unit test
namespace estd { namespace internal {

template<>
struct optional_base<SpecializedEnum> : layer1::optional_base<SpecializedEnum, 4>
{
    typedef layer1::optional_base<SpecializedEnum, 4> base_type;

    ESTD_CPP_FORWARDING_CTOR(optional_base)
};

}}


TEST_CASE("optional")
{
    SECTION("simple")
    {
        optional<int> val(5);
        //optional<int> val;

        //val = 5;

        REQUIRE(val.has_value());
    }
    SECTION("simple 2")
    {
        optional<int> val;

        REQUIRE(!val);

        val = 5;

        REQUIRE(val);
    }
    SECTION("null option")
    {
        optional<int> val(nullopt);

        REQUIRE(!val);

        val = 5;

        REQUIRE(val);
        REQUIRE(*val == 5);

        val = nullopt;

        REQUIRE(!val);
    }
    SECTION("looking for tag")
    {
        REQUIRE(internal::optional_tag_base::has_optional_tag_typedef<internal::optional_tag_base>::value);

        REQUIRE(optional<int>::has_optional_tag_typedef<internal::optional_tag_base>::value);
        REQUIRE(optional<int>::has_optional_tag_typedef<layer1::optional<int>>::value);

        REQUIRE(layer1::optional<int>::has_optional_tag_typedef<internal::optional_tag_base>::value);
        REQUIRE(layer1::optional<int>::has_optional_tag_typedef<optional<int>>::value);

        REQUIRE(!layer1::optional<int>::has_optional_tag_typedef<int>::value);
    }
    SECTION("layer1 version")
    {
        layer1::optional<int, -1> val;

        REQUIRE(*val == -1);

        REQUIRE(!val);

        val = 5;

        REQUIRE(val.has_value());

        int v = *val;

        REQUIRE(v == 5);

        REQUIRE(val.null_value == -1);

        SECTION("conversion to traditional")
        {
            optional<int> val2(val);
            layer1::optional<int> val3(val2);

            REQUIRE(val2);
            REQUIRE(val3);
            REQUIRE(*val2 == 5);
            REQUIRE(*val3 == 5);

            int sz = sizeof(val);

            REQUIRE(sz == sizeof(int));

            val = 3;

            val2 = val;

            REQUIRE(val2);

            int v = *val2;

            REQUIRE(v == 3);
        }
        SECTION("nullopt")
        {
            val = nullopt;

            REQUIRE(!val);

            layer1::optional<int, -1> val(nullopt);

            REQUIRE(!val);

            val = 5;

            optional<int> val2;

            // NOTE: according to spec, this is a semi-valid operation,
            // though leaning on the NO because we haven't set has_value to true this way
            //val2.value() = 1;     // Doesn't work due to bad_option_exception
            *val2 = 1;

            REQUIRE(!val2);

            val = val2;

            REQUIRE(!val);
        }
        SECTION("assign from bitfield")
        {
            BitField bf;

            bf.val2 = 10;

            val = (int)bf.val2;

            REQUIRE(val.has_value());
            REQUIRE(val == bf.val2);
        }
        SECTION("rvalue")
        {
            enum Options { Register, Unregister, Unspecified = 3 };
            typedef estd::layer1::optional<Options, Unspecified> option_value_type;

            struct T1
            {
                Options o : 2;

                option_value_type opt() const { return o; }
            };
            T1 t1{Unregister};
            option_value_type v2 = t1.opt().value();

            REQUIRE(v2 == Unregister);
        }
    }
    SECTION("comparisons / equality")
    {
        SECTION("int")
        {
            estd::optional<int> val;

            suite(val, 4);

            val = 5;

            REQUIRE(val > 4);
            REQUIRE(val == 5);
        }
        SECTION("bool")
        {
            estd::optional<bool> val;

            suite(val, true);
        }
        SECTION("layer1: int")
        {
            estd::layer1::optional<int> val, val3;

            suite(val, 4);

            val = 5;

            REQUIRE(val > 4);
            REQUIRE(val == 5);
            REQUIRE(val < 10);
            REQUIRE(val <= 10);

            int val2 = -1;

            val = val2;

            REQUIRE(val);

            val2 = 0;
            val = val2;

            // layer1 flavor in this configuration means 0 == null
            REQUIRE(!val);
            REQUIRE(val == val3);

            try
            {
                val.value();

                FAIL();
            }
            catch(const bad_optional_access&)
            {

            }

            SECTION("unusual value")
            {
                layer1::optional<int16_t, 0x1000> o;

                REQUIRE(*o == 0x1000);
            }
        }
        SECTION("layer1: bool")
        {
            estd::layer1::optional<bool> val;

            suite(val, false);

            constexpr int sz = sizeof(val);

            REQUIRE(sz == sizeof(bool));
        }
        SECTION("layer1: enum")
        {
            estd::layer1::optional<PlainOldEnum, POE_UNINITIALIZED> o;

            o.reset();
        }
    }
    SECTION("function interaction/return value")
    {
        SECTION("standard")
        {
            estd::optional<int> value = returning_optional(5);

            REQUIRE(value);
            REQUIRE(*value == 5);

            value = returning_optional(10);

            REQUIRE(!value);
        }
        SECTION("layer1")
        {
            estd::layer1::optional<int, -1> value = returning_optional(5);

            REQUIRE(value);
            REQUIRE(value.value() == 5);

            value = returning_optional(10);

            REQUIRE(!value);
            REQUIRE(*value == -1);
        }
    }
    SECTION("auto specializing based on traits/base")
    {
        estd::optional<SpecializedEnum> o;

        REQUIRE(o.bitsize() == 4);
        REQUIRE(o.has_value() == false);

        o = Value1;

        REQUIRE(o == Value1);

        o.reset();

        // DEBT: MinGW 64-bit always produces a size of 8 bytes, even with a bit packed
        // struct of only 5 bits.  Not sure why that is.  Feels reasonable on a 64-bit architecture,
        // but marking this as debt until we find out precisely why
#ifndef __MINGW64__
        REQUIRE(sizeof(o) == sizeof(SpecializedEnum));
#endif
    }
    SECTION("non trivial")
    {
        estd::optional<test::NonTrivial> o;
        int counter = 0;

        REQUIRE(o.has_value() == false);

        o.emplace(7, [&]{ ++counter; });

        REQUIRE(o.has_value());
        REQUIRE(o->code_ == 7);
        REQUIRE(o->copied_ == false);
        REQUIRE(counter == 0);

        o.reset();

        REQUIRE(counter == 1);

        o.emplace(77, [&]{ ++counter; });

        SECTION("constructor")
        {
            optional<test::NonTrivial> o2(o), o_empty;
            optional<test::NonTrivial> o_empty2(o_empty);

            SECTION("move")
            {
                optional<test::NonTrivial> o3(std::move(o2));

                REQUIRE(o3->initialized_);
                REQUIRE(o3->copied_ == false);
                REQUIRE(o3->moved_);
            }

            REQUIRE(counter == 2);
        }
        SECTION("assignment")
        {
            optional<test::Dummy> o2(in_place_t{}, 7, "hi2u"), o3;

            // NonTrivial lacks assignment operator on purpose
            //optional<test::NonTrivial> o2;

            REQUIRE(!o2->copied_);
            REQUIRE(!o2->moved_);

            o3 = o2;

            // DEBT: Depending on 'noexcept' specifier, optional will create
            // a temporary along the way and move from it - this is (probably)
            // unwanted behavior inherited by variant_storage
            REQUIRE(!o3->copied_);
            REQUIRE(o3->moved_);

            SECTION("move")
            {
                o2 = std::move(o3);

                REQUIRE(o2->moved_);
            }
        }
    }
    SECTION("type checks")
    {
        optional<bool> v1;
        layer1::optional<bool> v2;
        optional<int> v3;

        using type1_1 = decltype(optional<bool>{}.value());
        using type1_2 = decltype(layer1::optional<bool>{}.value());
        using type1_3 = decltype(optional<int>{}.value());

        using type2_1 = decltype(v1.value());
        using type2_2 = decltype(v2.value());

        REQUIRE(is_rvalue_reference<type1_1>::value);
        REQUIRE(!is_rvalue_reference<type1_2>::value);
        REQUIRE(is_rvalue_reference<type1_3>::value);

        REQUIRE(is_lvalue_reference<type2_1>::value);
        REQUIRE(!is_lvalue_reference<type2_2>::value);
    }
    SECTION("hash")
    {
        using type = layer1::optional<uint16_t, 0xFFFF>;
        using hash = estd::hash<type>;

        type v1, v2(5);

        REQUIRE(hash{}(v1) == type::null_value);
        REQUIRE(hash{}(v2) == 5);
    }
    SECTION("internal")
    {
        // Tucked away in internal namespace is a full bitwise flavor of optional.  So far this is
        // only used to layer1 bool specialization
        SECTION("bitwise")
        {
            using bitwise = estd::optional<int, estd::internal::layer1::optional_base<int, 10, 0> >;
            using value_type = decltype(bitwise{}.value());

            REQUIRE(!is_rvalue_reference<value_type>::value);

            struct V
            {
                int b_ : 10;

                bitwise b() const { return b_; }
            } v{5};
            bitwise b1;

            b1 = 5;

            REQUIRE(v.b().has_value());

            int v2 = v.b().value();

            REQUIRE(v2 == 5);

            // FIX: Not working
            //REQUIRE(v.b() == b1);
        }
    }
}
