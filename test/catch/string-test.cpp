#include <estd/string.h>
#include <estd/string_view.h>
#include <estd/vector.h>
#include <cstdlib>
#include <ostream>

#include "mem.h"

using namespace estd;
//using namespace estd::experimental;

namespace std
{
#ifdef UNUSED

template <class TChar, class TCharTraits, class TAllocator>
std::ostream& operator <<( std::ostream& os, estd::basic_string<TChar, TCharTraits, TAllocator> const& value)
{
    int len = value.size();
    /*
    TChar* s = value.lock();
    // because we can't be sure of null termination without doing more checks

    while(len--) os << *s++;

    value.unlock(); */

    char buf[256];
    char* s = buf;

    value.copy(buf, sizeof(buf));

    while(len--) os << *s++;

    return os;
}
#else

/*
template <class TChar, class Traits, class TAllocator, class TStringTraits>
std::basic_ostream<TChar, Traits>& operator <<(
        std::basic_ostream<TChar, Traits>& os,
        const estd::basic_string<TChar, Traits, TAllocator, TStringTraits>& value)
{
    return ::operator <<(os, value);
}
*/

template <class TChar, class TStringTraits, class TAllocator>
std::ostream& operator <<( std::ostream& os,
                           const estd::basic_string<TChar, typename TStringTraits::char_traits, TAllocator, TStringTraits>& value)
{
    return ::operator <<(os, value);
}
#endif
}


#include <catch.hpp>


TEST_CASE("string tests")
{
    SECTION("string tests")
    {
        basic_string<char, std::char_traits<char>, _allocator<char>> test;

        test += "hello";
    }
    SECTION("A")
    {
        //string val;
        basic_string<char, std::char_traits<char>, _allocator<char>> val;

        char buf[128];

        val += "hello";

        val.copy(buf, 128);

        REQUIRE(buf[0] == 'h');
        REQUIRE(buf[1] == 'e');
        REQUIRE(buf[2] == 'l');
        REQUIRE(buf[3] == 'l');
        REQUIRE(buf[4] == 'o');
    }
    SECTION("layer 1 null terminated")
    {
        layer1::basic_string<char, 20> s;
        int sz = sizeof(s);

        REQUIRE(s.length() == 0);

        s += "hello";

        REQUIRE(s.length() == 5);
        REQUIRE(s == "hello");
    }
    SECTION("layer 2 (non-experimental) null terminated")
    {
        char buf[128] = ""; // as per spec, this actually
        layer2::basic_string<char, 20> s(buf, 0);
        //layer2::basic_string<const char, 20> s2 = "hi";

        int sz = sizeof(s);

        REQUIRE(s.length() == 0);

        s += "hello";

        REQUIRE(s.length() == 5);
        REQUIRE(s == "hello");
    }
    // deactivated because string construction for layer2 is still a bit funky
    // and depends on dynamic_array_helper.  dynamic_array_helper code needs a
    // cleanup, so do that first, then fix this
    SECTION("layer 2 null terminated")
    {
        char buf[128];
        layer2::basic_string<const char, 4> str("val");

        str.copy(buf, 128);

        REQUIRE(buf[0] == 'v');
        REQUIRE(buf[1] == 'a');

        REQUIRE(str.size() == 3);
        REQUIRE(str.max_size() == 3);

        layer2::basic_string<const char, 0> str2 = str;

        REQUIRE(str2.size() == 3);
        REQUIRE(str == str2);
        REQUIRE(str == "val");

        //experimental::layer2::string<> str3 = str;
    }
    SECTION("layer 3 null terminated")
    {
        char buf[128];
        char buf2[128] = "val";
        layer3::basic_string<char> str(buf2, 3);

        str.copy(buf, 128);

        REQUIRE(buf[0] == 'v');
        REQUIRE(buf[1] == 'a');

        REQUIRE(str.size() == 3);
        REQUIRE(str.max_size() == 127);

        // FIX: For C++98 compat, we still have to associate the 'parity' version of things
        // and since we don't, there's a redundant max-sizing in here of 0
        str += ":hi2u";

        REQUIRE(str.size() == 8);

        REQUIRE(buf2[3] == ':');
        REQUIRE(buf2[7] == 'u');

    }
    SECTION("dynamic_array low level allocator test")
    {
        //estd::vector<uint8_t, test_t> d;
        estd::vector<uint8_t, internal::single_fixedbuf_allocator<uint8_t, 10>> d;

        d.push_back(3);

        uint8_t v = d[0];
        auto vv = (int)v;

        // flaky sometimes, likely due to underlying locking magic
        REQUIRE(v == 3);
        REQUIRE(vv == 3);
    }
    SECTION("single_fixedbuf_allocator")
    {
        estd::basic_string<char, std::char_traits<char>, internal::single_fixedbuf_allocator<char, 30>> s;
        char buf[128];

        s += "hello";

        buf[s.copy(buf, 128)] = 0;

        REQUIRE(strcmp(buf, "hello") == 0);
    }
    SECTION("single_nullterm_fixedbuf_allocator")
    {
        estd::basic_string<char, std::char_traits<char>, internal::single_fixedbuf_allocator<char, 30, true>> s;
        int sz = sizeof(s);
        char buf[128];

        REQUIRE(s.size() == 0);

        s += "hello";

        REQUIRE(s == "hello");

        REQUIRE(s.size() == 5);

        buf[s.copy(buf, 128)] = 0;

        REQUIRE(strcmp(buf, "hello") == 0);
    }
    // see 'layer 2 null terminated' section for comments
    SECTION("layer2 -> layer3 promotion")
    {
        layer2::basic_string<const char, 10> val = "hello";
        layer3::basic_string<const char> val2 = val;
        layer1::string<128> val3;

        int sz1 = sizeof(val);
        int sz2 = sizeof(val2);

        REQUIRE(sz1 == sizeof(char*));
        REQUIRE(sz2 == sizeof(char*) + sizeof(size_t));

        char buf[128];

        buf[val2.copy(buf, 128)] = 0;

        val3 = val;

        REQUIRE(val3 == "hello");
        REQUIRE(val3 == buf);
    }
    SECTION("layer3 null terminated")
    {
        char buf[128];

        layer3::basic_string<char> s(buf, 0);

        REQUIRE(s.size() == 0);

        s += "hello";

        REQUIRE(s.size() == 5);
        REQUIRE(s == "hello");
    }
    SECTION("layer3 length-specified (not null terminated)")
    {
        char buf[128];

        memset(buf, 0, sizeof(buf));

        layer3::basic_string<char, false> s(buf, 0);

        REQUIRE(s.size() == 0);

        s += "hello";

        REQUIRE(s.size() == 5);
        REQUIRE(s == "hello");

        // since buf was zeroed out beforehand, we can treat it
        // as a C-style string
        layer3::basic_string<char, false> s2(buf);

        REQUIRE(s == s2);

        layer1::string<20> s3(s2);

        REQUIRE(s3 == s);
    }
    SECTION("Non experimental conversion between layers")
    {
        char buf2[100];
        char buf3[100];

        layer1::basic_string<char, 100> s1;
        layer2::basic_string<char, 100> s2(buf2, 0);

        s1 = "Hello";

        REQUIRE(s1 == "Hello");

        s2 = s1;

        REQUIRE(s2 == "Hello");
        REQUIRE(s2 == s1);
    }
    SECTION("Aliases/typedefs for layers")
    {
        char buf2[100];
        char buf3[100];

        layer1::string<100> s1;

        // This invokes some kind of infinite loop program never ends
        // Filters down into map_base somehow... ??
        s1 = "Hello";

        layer2::string<> s2(buf2, 0);
        layer3::string s3(buf3, 0);

        s2 = s1;

        REQUIRE(s1 == "Hello");
        REQUIRE(s2 == "Hello"); // Fails due to string<> def above, but shouldn't (should grow unboundedly)

        s1 += s2;

        // this works if we get by s2 == hello test
        REQUIRE(s1 == "HelloHello");

        s3 = s1;

        REQUIRE(s3 == s1);

        s1 = s2;

        REQUIRE(s1 == "Hello");
    }
    SECTION("indexer: layer1")
    {
        layer1::string<20> s = "hi2u";

        REQUIRE(s == "hi2u");

        s[2] = '3';

        REQUIRE(s == "hi3u");
    }
    SECTION("String literal assignment")
    {
        // DOES work sort of, but many operations like .copy don't work right
        // because it is too const'd up
        const layer2::basic_string<const char, 0> s = "Hello World";

        layer2::basic_string<const char, 0> s2 = "Hello World";
        layer2::const_string s3 = "Hello World, again";

        char buf[128];

        buf[s2.copy(buf, sizeof(buf))] = 0;

        REQUIRE(strlen(buf) > 0);
        REQUIRE(s2 == "Hello World");
        REQUIRE(s3 == "Hello World, again");

        /*
        estd::basic_string<
                        char, std::char_traits<char>,
                        estd::experimental::single_fixedbuf_allocator < char, 0, true, char* > > s2 = "hello"; */
    }
    SECTION("Erase portion of string")
    {
        layer1::string<40> s = "Hello World";

        s.erase(1, 1);

        REQUIRE(s == "Hllo World");

        s.pop_back();

        REQUIRE(s == "Hllo Worl");
    }
    SECTION("String iteration")
    {
        layer1::string<40> s = "Hello World";

        int i = 0;

        for(auto c : s)
        {
            INFO("i = " << i);
            REQUIRE(s[i++] == c);
        }
    }
    SECTION("layer3 copy constructor")
    {
        layer2::basic_string<const char, 0> s = "Hello, World";
        layer3::basic_string<const char> s2 = s;

        // works, though underlying buffer_size is max-value
        // cause of N=0 above.  Tolerable, but I'd prefer it
        // to deduce the size from strlen in this case
        REQUIRE(s2 == "Hello, World");
    }
    SECTION("layer3 const compare")
    {
        layer3::const_string s("test");

        INFO("Evaluating" << s);
        REQUIRE(s.length() == 4);

        // because we've denoted basic_string as not null terminated.  But, because it's const,
        // we'll never get to use that extra byte
        REQUIRE(s.capacity() == 4);
        REQUIRE(s.max_size() == 4);

        REQUIRE(s == "test");

        SECTION("compare against layer1")
        {
            layer1::string<20> s2 = "test";

            REQUIRE(s == s2);
        }
    }
    SECTION("to_string int")
    {
        char buf[128];
        layer2::string<> s(buf, 0);

        REQUIRE(s.size() == 0);

        s += "The value is ";
        s += to_string(123);

        REQUIRE(s == "The value is 123");
    }
    SECTION("starts_with")
    {
        layer3::const_string s("test");

        REQUIRE(s.starts_with("te"));
        REQUIRE(!s.starts_with("st"));
    }
    SECTION("string_view")
    {
        string_view sv("test", 4);
        string_view sv2 = sv;
        string_view sv3 = "test3";

        int s = sizeof(sv);

        REQUIRE(sv3.starts_with(sv));
        REQUIRE(sv2 == sv);
        REQUIRE(sv2.compare(sv) == 0);
        REQUIRE(sv2 == "test");

        // FIX: Does not work yet - need to make helper/allocator shrink
        // the end of the buffer which is not something allocator normally
        // does (remember, we're treating allocator max_size as string_view
        // size, so a regular reallocation is a bit of an unknown)
        sv3.remove_suffix(2);
    }
}
