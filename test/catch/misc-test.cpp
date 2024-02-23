#include <catch.hpp>

#include <estd/array.h>
#include <estd/functional.h>
#include <estd/type_traits.h>
#include <estd/variant.h>
#include <estd/internal/container/set.h>
#include <estd/internal/container/unordered_set.h>

using namespace estd;

// For one-off tests not warranting its own test file
TEST_CASE("miscellaneous")
{
    SECTION("monostate")
    {
        REQUIRE(estd::is_empty_f<estd::monostate>());
        REQUIRE(monostate{} == monostate{});

        SECTION("hash")
        {
            estd::hash<monostate> hash_fn;

            REQUIRE(hash_fn(monostate{}) == hash_fn(monostate{}));
        }
    }
    SECTION("unordered_set")
    {
        using type = estd::internal::unordered_set<array<int, 10 > >;

        type value;
    }
}
