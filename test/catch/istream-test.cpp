#include <catch2/catch_all.hpp>

#include <estd/istream.h>
#include <estd/sstream.h>

// DEBT: As we fiddle with bringing string_view istreambuf online, we must manually
// include this
#include <estd/string_view.h>

using namespace estd;

TEST_CASE("istream")
{
    using streambuf_type = layer1::stringbuf<32>;

    SECTION("basics")
    {
        const layer1::string<32> str = "hi2u";

        detail::basic_istream<streambuf_type> _cin(str);

        SECTION("readsome")
        {
            char localbuf[128];
            estd::streamsize read_back = _cin.readsome(localbuf, str.length());

            REQUIRE(read_back == str.length());
            REQUIRE(localbuf[0] == str[0]);
        }
        SECTION("seekg")
        {
            _cin.seekg(1, estd::ios_base::cur);
            REQUIRE('i' == _cin.get());
        }
    }
    SECTION("parsing")
    {
        SECTION("integer conversion")
        {
            layer1::string<32> str{"2 xyz"};

            detail::basic_istream<streambuf_type> _cin(str);

            int val = - 1;

            _cin >> val;

            REQUIRE(_cin.good());
            REQUIRE(val == 2);

            _cin >> val;

            REQUIRE(val == 0);
            REQUIRE(_cin.good() == false);

            if(_cin) FAIL();
        }
        SECTION("integer conversion from hex")
        {
            layer2::basic_istringstream<const char>
                in("FF"),
                in2("4D2 A");

            unsigned v = 0;

            in >> hex;
            in >> v;

            REQUIRE(v == 255);

            in2 >> hex;
            in2 >> v;

            REQUIRE(v == 1234);

            in2 >> v;

            REQUIRE(v == 10);
        }
        SECTION("integer conversion from spanbuf")
        {
            detail::basic_ispanstream<const char> in("123 A");

            unsigned v = 0;

            in >> v;

            REQUIRE(v == 123);

            in >> hex >> v;

            REQUIRE(v == 10);
        }
        SECTION("integer overflow")
        {
            SECTION("signed")
            {
                layer1::string<32> str{"-70000"};

                detail::basic_istream<streambuf_type> _cin(str);

                int16_t v = 0;

                _cin >> v;

                REQUIRE(_cin.fail());
                REQUIRE(v == estd::numeric_limits<decltype(v)>::min());
            }
            SECTION("unsigned")
            {
                layer1::string<32> str{"2 70000"};
                detail::basic_istream<streambuf_type> _cin(str);

                uint16_t v = 0;

                _cin >> v;

                REQUIRE(_cin.eof() == false);
                REQUIRE(_cin.good());
                REQUIRE(v == 2);

                _cin >> v;

                REQUIRE(_cin.fail());
                REQUIRE(v == estd::numeric_limits<decltype(v)>::max());
            }
        }
        SECTION("istringstream")
        {
            layer1::istringstream<32> in = "123 456";

            int val;

            in >> val;

            REQUIRE(val == 123);

            in >> val;

            REQUIRE(val == 456);
        }
        SECTION("with const char")
        {
            const char* buf = "hi2u";

            SECTION("layer1 string")
            {
                using stream_type = layer2::basic_istringstream<const char>;
                stream_type in(buf);
                layer1::string<32> buf2;

                in >> buf2;

                REQUIRE(buf2 == buf);
            }
            SECTION("C-style string")
            {
                using stream_type = layer2::basic_istringstream<const char>;
                stream_type in(buf);
                char buf2[32];

                in >> buf2;

                REQUIRE(layer2::const_string(buf) == buf);
            }
            SECTION("const string")
            {
                using stream_type = layer2::basic_istringstream<const char>;
                layer2::const_string v("hello");
                stream_type in(v);
            }
            SECTION("read")
            {
                using stream_type = layer2::basic_istringstream<const char>;
                stream_type in("hello");

                char temp[32] {};

                in.read(temp, 5);

                REQUIRE(layer2::const_string(temp) == "hello");
            }
            SECTION("string view")
            {
                using stream_type = layer2::basic_istringstream<const char>;
                string_view v("Hello");
                stream_type in(v);

                char c = 0;

                in >> c;

                REQUIRE(c == 'H');

                in >> c;
                in >> c;
                in >> c;
                in >> c;

                REQUIRE(c == 'o');

                in >> c;

                REQUIRE(in.good() == false);

                /* ALmost, but not quite.  Doesn't play nice with a 'Container'
                 * constraint
                 *
                using stream_type = detail::basic_istream<
                    estd::internal::impl::v0::basic_sviewbuf<char>>;
                stream_type in(buf);
                layer1::string<32> buf2;

                in >> buf2;

                REQUIRE(buf2 == buf);
                */
            }
        }
    }
#if __cpp_deduction_guides
    SECTION("CTAD")
    {
        SECTION("ispanstream")
        {
            //span<const char, 4> s("hi2u");

            //detail::basic_ispanstream in(s);
        }
    }
#endif
    SECTION("cin")
    {
        // limited testing since an automated test shouldn't pause for input
        estd::istream _cin(*stdin);

        // POSIX in doesn't reveal in_avail
        //_cin.rdbuf()->in_avail();
    }
}
