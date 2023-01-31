#include <catch.hpp>

#include <estd/ostream.h>
#include <estd/sstream.h>
#include <estd/iomanip.h>

using namespace estd;

TEST_CASE("ostream")
{
    experimental::ostringstream<64> out;

    SECTION("output character")
    {
        out << 'A';

        REQUIRE(out.rdbuf()->str()[0] == 'A');
    }
    SECTION("output integer literal")
    {
        out << 7;

        REQUIRE(out.rdbuf()->str()[0] == '7');
    }
    SECTION("output integer variable")
    {
        int v = 7;

        out << v;

        REQUIRE(out.rdbuf()->str()[0] == '7');
    }
    SECTION("formatting")
    {
        out << setw(5);

        // TODO: Work with out_int_helper/write_int

        SECTION("fill")
        {
            out << setfill('0');

            SECTION("char")
            {
                out << 'H';

                auto s = out.rdbuf()->str();

                REQUIRE(s.length() == 5);

                REQUIRE(s[0] == '0');
            }
            SECTION("int")
            {
                out << 12;

                auto s = out.rdbuf()->str();

                REQUIRE(s[0] == '0');
            }
        }
        SECTION("setw")
        {
            SECTION("char")
            {
                out << 'H';

                auto s = out.rdbuf()->str();

                REQUIRE(s.length() == 5);
            }
            SECTION("int")
            {
                out << 12;

                auto s = out.rdbuf()->str();

                REQUIRE(s.length() == 5);
                REQUIRE(s[0] == ' ');
            }
        }
    }
}