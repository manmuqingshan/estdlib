//
// Created by malachi on 4/29/22.
//

#include <catch2/catch_all.hpp>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/string.h>
#include <estd/string_view.h>
#include <estd/sstream.h>

using namespace estd;

#include "macro/push.h"

using synthetic_streambuf_base = internal::impl::streambuf_base<estd::char_traits<char> >;

// DEBT: Continue fleshing out proper Streambuf concept... would be very handy
template <ESTD_CPP_CONCEPT(concepts::v1::OutStreambuf) Streambuf>
void true_buffered_test(Streambuf& sb)
{
    auto& str = sb.rdbuf().str();

    sb.sputc('h');

    REQUIRE(str.empty());

    sb.pubsync();

    REQUIRE(str[0] == 'h');

    sb.sputn("1234567", 7);

    REQUIRE(str.size() == 1);

    sb.sputc('8');

    REQUIRE(str.size() == 1);

    sb.sputc('9');

    REQUIRE(str.size() == 9);
}

TEST_CASE("streambuf")
{
    const char raw_str[] = "raw 'traditional' output\n";
    constexpr int raw_str_len = sizeof(raw_str) - 1;

    SECTION("basic_streambuf test")
    {
        // NOTE: posix_streambuf probably technically should be able to take stdin AND stdout...
        //posix_streambuf<char> psb(*stdout);
        //internal::basic_streambuf_wrapped<posix_streambuf<char>& > sbw(psb);

        internal::basic_streambuf_wrapped<posix_streambuf<char> > sbw(*stdout);
        basic_streambuf<char>& sb = sbw;

#ifdef TEST_COUT
        sb.sputn(raw_str, sizeof(raw_str) - 1);
#endif
    }
    SECTION("impl::out_span_streambuf")
    {
        // FIX: Not quite there because span constructor won't switch between Extent and
        // non Extent version for explicit buffer just yet
        constexpr int val_size = 32;
        uint32_t val[val_size];
        uint8_t test1[] = { 0, 1, 2, 3 };

        val[0] = 0;

        using streambuf_type = detail::basic_ospanbuf<uint8_t>;

        SECTION("initialization")
        {
            SECTION("array init")
            {
                uint8_t buffer[32];

                streambuf_type streambuf(buffer);

                streambuf.sputn((uint8_t*)"abc", 3);

                REQUIRE(streambuf.pos() == 3);
                REQUIRE(buffer[0] == 'a');
            }
        }
        SECTION("full (non impl) version")
        {
            // 'test' actually sits dormant and does nothing.  artifact of previous test
            // approach
            estd::internal::impl::out_span_streambuf<estd::char_traits<uint8_t>> test((uint8_t*)&val[0], 32);

            // NOTE: Wanted to do a ref version here but not sure if I actually
            // want that to be a supported technique
            // NOTE: This and many other of the '32' used around here are wrong, since
            // that's the uint32_t-oriented size
            detail::streambuf<decltype(test)> sb((uint8_t*)&val[0], 32);

            sb.sputc(0);
            sb.sputc(1);
            sb.sputc(2);
            sb.sputc(3);

            // TODO: Make this endian-inspecific.  For the time being though I anticipate
            // these unit tests only running on x64 machines
            REQUIRE(val[0] == 0x03020100);

            int sz = sizeof(sb);

            // TODO: Re-enable this, upgraded out_span_streambuf and broke old sizing
            //REQUIRE(sz == sizeof(estd::span<uint32_t, 32>) + sizeof(size_t));

            //REQUIRE(sb.epptr() - sb.pbase() == 32 * 4);
        }
        SECTION("non-constexpr size version")
        {
            streambuf_type sb((uint8_t*)&val[0], 32);

            sb.sputc(0);
            sb.sputc(1);
            sb.sputc(2);
            sb.sputc(3);

            // DEBT: Make this non-endian-specific
            REQUIRE(val[0] == 0x03020100);

            int sz = sizeof(sb);

            // TODO: Re-enable this, upgraded out_span_streambuf and broke old sizing
            // TODO: see why it grew from 24 to 32 bytes also (maybe because we inherit pos now
            // and before it was packing it? doubtful.  Probably because pos is this more complicated
            // pos_type from native char_traits)
            //REQUIRE(sz == sizeof(estd::span<uint32_t>) + sizeof(size_t));

            sb.pubseekoff(-1, estd::ios_base::cur);

            sb.sputc(4);

            // DEBT: Make this non-endian-specific
            REQUIRE(val[0] == 0x04020100);
        }
        SECTION("ostream usage (complex initialization)")
        {
            // Successfully cascades down 'val, 32' all the way down to out_span_streambuf
            detail::basic_ospanstream<uint32_t> out(val, 32);

            REQUIRE(out.rdbuf()->value().size() == 32);
        }
        SECTION("experimental")
        {
            SECTION("setbuf")
            {
                streambuf_type sb((uint8_t*)&val[0], val_size);

                uint8_t* data = sb.pptr();

                // 32-bit 0 will not match 8-bit 0, 1, 2, 3 on 2nd byte
                // regardless of endianess (unless we have some flavor of
                // the double-swap little endian looking like 1, 0, 3, 2)
                REQUIRE(data[1] != test1[1]);

                sb.setbuf_experimental(test1, sizeof test1);

                data = sb.pptr();

                REQUIRE(data[0] == test1[0]);
                REQUIRE(data[1] == test1[1]);
                REQUIRE(data[2] == test1[2]);
                REQUIRE(data[3] == test1[3]);
            }
        }
    }
    SECTION("in_span_streambuf")
    {
        char buf[128];

        using sb_type = estd::detail::basic_ispanbuf<const char>;

        typedef typename sb_type::traits_type traits_type;

        sb_type sb(raw_str);

        SECTION("sgetn")
        {
            estd::streamsize read_back = sb.sgetn(buf, sizeof(buf));

            REQUIRE(sb.eback() == raw_str);
            // Remember, span streambuf is not a string streambuf, so we get the null
            // termination back also
            REQUIRE(read_back == raw_str_len + 1);
        }
        SECTION("sbumpc")
        {
            REQUIRE(sb.sbumpc() == traits_type::to_int_type(raw_str[0]));
            REQUIRE(sb.sbumpc() == traits_type::to_int_type(raw_str[1]));
            REQUIRE(sb.sbumpc() == traits_type::to_int_type(raw_str[2]));
        }
        SECTION("pubseekoff")
        {
            int new_pos;

            new_pos = sb.pubseekoff(5, ios_base::cur);
            REQUIRE(new_pos == 5);
            new_pos = sb.pubseekoff(5, ios_base::cur);
            REQUIRE(new_pos == 10);
            REQUIRE(sb.sgetc() == traits_type::to_int_type(raw_str[new_pos]));
        }
        SECTION("pubseekpos")
        {
            int new_pos;

            new_pos = sb.pubseekpos(5);
            REQUIRE(new_pos == 5);
            REQUIRE(sb.sgetc() == traits_type::to_int_type(raw_str[new_pos]));
        }
        SECTION("sungetc")
        {
            sb.sbumpc();
            sb.sbumpc();
            sb.sbumpc();
            REQUIRE(sb.sungetc() == traits_type::to_int_type(raw_str[2]));
            REQUIRE(sb.sungetc() == traits_type::to_int_type(raw_str[1]));
            REQUIRE(sb.sungetc() == traits_type::to_int_type(raw_str[0]));
            REQUIRE(sb.sungetc() == traits_type::eof());
        }
    }
    SECTION("stringbuf")
    {
        SECTION("layer1 null terminated")
        {
            layer1::stringbuf<32> sb1;

            sb1.sputn("hi2u", 4);

            SECTION("gptr")
            {
                const char* v = sb1.gptr();

                REQUIRE(layer2::const_string(v) == "hi2u");
            }
            SECTION("seekpos")
            {
                long pos = sb1.pubseekpos(1, ios_base::in);

                REQUIRE(pos == 1);
                REQUIRE(layer2::const_string(sb1.gptr()) == "i2u");
            }
            SECTION("seekoff")
            {
                long pos = sb1.pubseekoff(1, ios_base::cur, ios_base::in);

                REQUIRE(pos == 1);
                REQUIRE(layer2::const_string(sb1.gptr()) == "i2u");
            }
        }
        SECTION("layer2")
        {
            layer2::basic_stringbuf<const char> sb1("hello");

            SECTION("gptr")
            {
                const char* v = sb1.gptr();

                REQUIRE(estd::layer2::const_string(v) == "hello");
            }
            SECTION("sgetn")
            {
                char temp[32];

                int count = sb1.sgetn(temp, 32);

                REQUIRE(count == 5);
                // FIX: span-ish behavior not quite working here
                //REQUIRE(layer2::string<5, false>(temp) == "hello");
                REQUIRE(layer2::string<>(temp, 5) == "hello");
            }
        }
    }
    SECTION("pubsync - method finding")
    {
        struct pubsync_only_streambuf_impl : synthetic_streambuf_base
        {
            int sync() const { return 7; }
        };

        layer1::stringbuf<32> sb1;

        internal::streambuf<pubsync_only_streambuf_impl> sb2;

        REQUIRE(sb1.pubsync() == 0);
        REQUIRE(sb2.pubsync() == 7);
    }
    SECTION("ambiguous character availability")
    {
        struct synthetic_unavail_streambuf_impl : synthetic_streambuf_base
        {
            typedef synthetic_streambuf_base base_type;
        };

        internal::streambuf<synthetic_unavail_streambuf_impl> sb;

        REQUIRE(sb.in_avail() == 0);
    }
    SECTION("'true' streambuf")
    {
        using backing = layer1::stringbuf<256>;

        //internal::impl::layer1::bipbuf<16> bb;

        //bb.offer_begin();

        SECTION("buffered stringbuf")
        {
            using type = internal::out_buffered_stringbuf<backing, 8>;
            type sb;
            auto& str = sb.rdbuf().str();

            true_buffered_test(sb);
        }
        SECTION("out bipbuffer (layer1)")
        {
            backing sbb;
            using type = internal::out_bipbuf_streambuf<backing&, 8>;
            type sb(sbb);
            auto& str = sb.rdbuf().str();

            true_buffered_test(sb);
        }
        SECTION("out bipbuffer (layer3)")
        {
            backing sbb;
            estd::layer1::bipbuf<8> bb;
            using type = internal::out_bipbuf_streambuf<backing&, 0>;
            type sb(bb.native(), sbb);

            true_buffered_test(sb);
        }
        SECTION("in bipbuffer")
        {
            backing sbb;
            using type = internal::in_bipbuf_streambuf<backing&, 8>;
            type sb(sbb);

            REQUIRE(sb.sgetc() == -1);
            sb.sbumpc();

            sbb.sputn("hi2u", 4);

            REQUIRE(sb.sbumpc() == 'h');
            REQUIRE(sb.in_avail() == 3);
        }
    }
}

#include "macro/pop.h"
