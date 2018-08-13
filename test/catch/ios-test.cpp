//
// Created by malachi on 8/10/18.
//

#include <catch.hpp>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/string.h>

using namespace estd;

TEST_CASE("iostreams")
{
    SECTION("SFINAE tests")
    {
        typedef estd::internal::impl::native_streambuf<char, estd::TEST_STREAM_T, std::char_traits<char> >
                streambuf_impl_type;
        typedef estd::internal::streambuf<streambuf_impl_type> streambuf_type;
        bool has_method = streambuf_type::has_sputc_method<streambuf_impl_type>::value;

        REQUIRE(has_method);
    }
    SECTION("cin")
    {
        // limited testing since an automated test shouldn't pause for input
        estd::istream _cin(*stdin);

        // POSIX in doesn't reveal in_avail
        //_cin.rdbuf()->in_avail();
    }
    SECTION("cout")
    {
        //estd::basic_streambuf<char> streambuf(*stdout);
        estd::ostream _cout(*stdout);

        int value = 123;

        _cout << "Got here #";
        _cout << value;
        _cout << estd::endl;

        //_cout.put('!');
        _cout << '!';

        SECTION("estd::string")
        {
            layer1::string<20> s = "hi";

            _cout << s << endl;
        }
    }
}
