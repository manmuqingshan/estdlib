//
// Created by malachi on 8/10/18.
//

#include <catch2/catch_all.hpp>

#include <condition_variable>
#include <future>

#include <estd/istream.h>
#include <estd/iterator.h>
#include <estd/ostream.h>
#include <estd/string.h>
#include <estd/sstream.h>

// TODO: We need a better place to locate specialized overloads of << [this one
// is for the dynamic_array handler for istream]
#include <estd/internal/istream.h>
#include <estd/internal/ostream.h>

#include "test-data.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#if __has_warning("-Wunused-but-set-variable")
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif


using namespace estd;

//#define TEST_COUT

struct dummy_streambuf_impl : internal::impl::streambuf_base<std::char_traits<char> >
{
    using base_type = internal::impl::streambuf_base<std::char_traits<char> >;
#if FEATURE_ESTD_STREAMBUF_TRAITS
    struct traits_type : base_type::traits_type
    {
        // this particular type gets lifted and instantiated in hosting istream/ostream
        // only.  (no auto signaling with a bare streambuf, though you can manually set
        // up this 'signal' in theory)
        struct signal
        {
            // NOTE: It's not likely you want multiple copies of an istream/ostream.
            // just hedging bets here.  Unsure how we should truly deal with that scenario.
            // Multiple copies of the streambuf, well, that's on you
            signal* next = nullptr;

            static constexpr std::chrono::milliseconds timeout { 100 };

            // DEBT: it's likely we'll want distinct mutex/cv for each, but
            // most of our use cases are istream OR ostream, not the combo
            std::mutex m;
            std::condition_variable cv;
            bool dtr_ = false;
            bool cts_ = false;

            // TODO: add timeouts in parameter lists

            bool wait_dtr()
            {
                std::unique_lock lk(m);
                return cv.wait_for(lk, timeout, [&]{ return dtr_; });
            }

            bool wait_cts()
            {
                std::unique_lock lk(m);
                return cv.wait_for(lk, timeout, [&]{ return cts_; });
            }

            void set_dtr()
            {
                {
                    std::unique_lock lk(m);
                    dtr_ = true;
                }
                cv.notify_one();
            }

            void set_cts()
            {
                {
                    std::unique_lock lk(m);
                    cts_ = true;
                }
                cv.notify_one();
            }
        };
    };

    traits_type::signal* signal_;

    // DEBT: Sets signal pointer - clumsy, but acceptable.  Would prefer RAII, but that's
    // a huge commitment across all the streambuf constructors
    void add_signal(traits_type::signal* v)
    {
        if(signal_) v->next = signal_;
        signal_ = v;
    }

    void del_signal(traits_type::signal* v) {}

#else
    using typename base_type::traits_type;
#endif

    typedef char char_type;
    typedef int int_type;

    // Make believe circular buffer (perhaps now use actual bipbuffer)

    int xsgetn(char_type*, int len)
    {
#if FEATURE_ESTD_STREAMBUF_TRAITS
        // make believe empty out part of circular buffer, signal clear to send
        signal_->set_cts();
#endif
        return len;
    }

    int xsputn(const char_type*, int len) { return len; }
    int sputc(char_type) { return 0; }
};

typedef internal::streambuf<dummy_streambuf_impl> dummy_streambuf;

TEST_CASE("ios")
{
    const char raw_str[] = "raw 'traditional' output\n";
    constexpr int raw_str_len = sizeof(raw_str) - 1;

    /* This old cute and clever function-detector method no longer employed.
     * Ended up being more complicated than not using it in the end
    SECTION("SFINAE tests")
    {
        //typedef estd::internal::impl::native_streambuf<char, estd::internal::posix_stream_t, std::char_traits<char> >
        //        streambuf_impl_type;
        typedef estd::internal::impl::posix_streambuf streambuf_impl_type;
        typedef estd::internal::streambuf<streambuf_impl_type> streambuf_type;
        bool has_method = streambuf_type::has_sputc_method<streambuf_impl_type>::value;

        REQUIRE(has_method);
    } */
    SECTION("experimental tests")
    {
        SECTION("wrap/convert")
        {
            detail::basic_ostream<dummy_streambuf> _cout;

            auto wrapped_out = experimental::convert(_cout);
            ostream& out = wrapped_out;
        }
        // TODO: Has made it past experimental phase, move out of experimental area
        SECTION("flagged istream")
        {
            SECTION("implicit")
            {
                estd::internal::flagged_istream<dummy_streambuf> _in;
                REQUIRE(decltype(_in)::policy_type::blocking() == false);
            }
            SECTION("explicit")
            {
                // DEBT: Something odd about Catch is it has trouble doing a REQUIRE against
                // struct member constexpr variables
                unsigned _blocking = estd::internal::istream_flags::blocking;
                unsigned _runtime_blocking = estd::internal::istream_flags::runtime_blocking;

                SECTION("blocking")
                {
                    estd::internal::flagged_istream<dummy_streambuf,
                        estd::internal::istream_flags::blocking> _in;
                    REQUIRE(_in.policy().blocking() == _blocking);
                }
                SECTION("combination 1")
                {
                    estd::internal::flagged_istream<dummy_streambuf,
                        estd::internal::istream_flags::non_blocking |
                        estd::internal::istream_flags::traditional_rdbuf
                        > _in;
                    REQUIRE(_in.policy().blocking() == false);
                }
                SECTION("combination 2")
                {
                    estd::internal::flagged_istream<dummy_streambuf,
                        estd::internal::istream_flags::runtime_blocking |
                        estd::internal::istream_flags::traditional_rdbuf
                        > _in;
                    REQUIRE(decltype(_in)::policy_type::blocking() == _runtime_blocking);
                }
            }
        }
    }
    SECTION("internal basic_stringbuf test")
    {
        typedef internal::impl::basic_stringbuf<layer1::string<32> > impl_type;
        typedef internal::streambuf<impl_type> streambuf_type;
        typedef typename streambuf_type::traits_type traits_type;

        // TODO: Move the raw stringbuf portion out to streambuf-test
        SECTION("impl")
        {
            impl_type sb;

            sb.xsputn(raw_str, sizeof (raw_str) - 1);

            REQUIRE(sb.str() == raw_str);
        }
        SECTION("internal")
        {
            streambuf_type sb;

            sb.sputn(raw_str, sizeof (raw_str) - 1);

            REQUIRE(sb.str() == raw_str);
        }
        SECTION("sungetc")
        {
            streambuf_type sb(raw_str);

            sb.sbumpc();
            sb.sbumpc();
            sb.sbumpc();
            REQUIRE(sb.sungetc() == raw_str[2]);
            REQUIRE(sb.sungetc() == raw_str[1]);
            REQUIRE(sb.sungetc() == raw_str[0]);
            REQUIRE(sb.sungetc() == traits_type::eof());
        }
        SECTION("ostream / istream")
        {
            char localbuf[128];
            layer1::string<32> str = "hi2u";

            // wrap with a (basically) real ostream
            detail::basic_ostream<streambuf_type> _cout;
            // extract the internal inline-value rdbuf
            streambuf_type* rdbuf = _cout.rdbuf();

            _cout << raw_str;

            REQUIRE(_cout.rdbuf()->str() == raw_str);

            //_cout.rdbuf()->str().clear();

            _cout << '!';
            _cout << str;

            // wrap reference to streambuf with (basically) real istream
            detail::basic_istream<streambuf_type&> _cin(*rdbuf);

            // pull data out, using same rdbuf as _cout
            _cin.read(localbuf, raw_str_len);

            //localbuf[10] = 0;

            //REQUIRE(localbuf[0] == raw_str[0]);
            REQUIRE(memcmp(localbuf, raw_str, raw_str_len) == 0);

            // FIX: Doesn't work, because internal pointers don't advance
            REQUIRE(_cin.get() == '!');
            //_cin >> localbuf;
        }
        SECTION("whitespace on input")
        {
            // wrap with a (basically) real ostream
            detail::basic_ostream<streambuf_type> _cout;
            // extract the internal inline-value rdbuf
            streambuf_type* rdbuf = _cout.rdbuf();
            // wrap reference to streambuf with (basically) real istream
            detail::basic_istream<streambuf_type&> _cin(*rdbuf);

            layer1::string<32> str;
            layer1::string<32> whitespace_str = "   ";

            // TODO: organize this, << operator is organized totally differently than >>
            // operator (<< is in string.h, >> is in internal/istream.h)
            _cout << whitespace_str;
            _cout << "     lots of whitespace!  ";

            //_cin >> estd::ws;
            _cin >> str;

            const char* helper = str.clock();

            REQUIRE(str == "lots");

            _cin >> str;

            helper = str.clock();

            REQUIRE(str == "of");

            _cin >> str;

            helper = str.clock();

            REQUIRE(str == "whitespace!");

            REQUIRE(_cin.good());

            _cin >> str;

            REQUIRE(_cin.eof());

            _cin.sync();
        }
        SECTION("basic_streambuf_wrapped")
        {
            streambuf_type sb;
            internal::basic_streambuf_wrapped<streambuf_type&> sbw(sb);
            estd::basic_ostream<char> _cout(&sbw);

            _cout << "hi2u";

            const char* helper = sb.str().data();

            REQUIRE(sb.str() == "hi2u");
        }
        SECTION("wrapped_ostream")
        {
            streambuf_type sb;
            detail::basic_ostream<streambuf_type&> native_cout(sb);
            experimental::wrapped_ostream<streambuf_type&> _cout(sb);
            ostream& __cout = _cout;

            __cout << "hi2u";

            const char* helper = sb.str().data();

            REQUIRE(sb.str() == "hi2u");

            auto wrapped_out = experimental::convert(native_cout);
        }
    }
    SECTION("cout")
    {
        //estd::basic_streambuf<char> streambuf(*stdout);
        estd::posix_ostream<char> _cout(*stdout);

#ifdef TEST_COUT

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
#endif
    }
    SECTION("layer1")
    {
        SECTION("stringbuf")
        {
            layer1::stringbuf<32> sb;
        }
        SECTION("ostringstream")
        {
            layer1::ostringstream<32> out;

            int sz = sizeof(out);

            out << "hi2u";

            REQUIRE(out.rdbuf()->str() == "hi2u");

            out << endl;
        }
        SECTION("istringstream")
        {
            layer1::istringstream<32> in = "hi2u";

            REQUIRE(in.rdbuf()->str() == "hi2u");
        }
        SECTION("numeric test")
        {
            layer1::ostringstream<32> out;

            SECTION("base 10")
            {
                out << dec; // NOTE: This is default, but we want to be thorough

                int value = 2;

                out << "hi" << value++ << 'u';

                auto& s = out.rdbuf()->str();

                // NOTE: Works here but in ASF/Atmel land the 'value' gets
                // treated as a character
                REQUIRE(s == "hi2u");
            }
            SECTION("base 16")
            {
                out << hex;

                out << "hi" << 15 << 'u';

                auto& s = out.rdbuf()->str();

                // NOTE: Works here but in ASF/Atmel land the 'value' gets
                // treated as a character
                REQUIRE(s == "hifu");
            }
        }
        SECTION("tellp")
        {
            layer1::ostringstream<32> out;
            int value = 2;

            out << "hi" << value++ << 'u';

            int tellp = out.tellp();

            REQUIRE(tellp == 4);
        }
    }
    SECTION("layer2")
    {
        SECTION("stringbuf")
        {
            //layer2::stringbuf(raw_str);
        }
    }
    SECTION("spitting out various strings")
    {
        layer1::ostringstream<256> out;

        SECTION("layer2")
        {
            estd::layer2::const_string s = "hi2u";

            out << s;

            REQUIRE(out.rdbuf()->str().size() == 4);
        }
        SECTION("layer3")
        {
            estd::layer3::const_string s = "hi2u";

            out << s;

            REQUIRE(out.rdbuf()->str().size() == 4);
        }
    }
    SECTION("convenience typedefs for span streaming")
    {
        char buf[128];
        estd::span<char> span(buf);

        SECTION("in")
        {
            strcpy(buf, "hello\r\n");
            estd::layer1::string<32> s;

            estd::detail::ispanstream in(span);

            in >> s;

            REQUIRE(s.starts_with("hello"));
        }
    }
#if FEATURE_ESTD_STREAMBUF_TRAITS
    SECTION("signaling")
    {
        detail::basic_ostream<dummy_streambuf> out;
        dummy_streambuf& rdbuf = *out.rdbuf();

        std::future<int> v = std::async(std::launch::async, [&]
        {
            return rdbuf.sgetn(nullptr, 1);
        });

        bool cts = out.signal().wait_cts();
        REQUIRE(cts);
        REQUIRE(v.wait_for(out.signal().timeout)==std::future_status::ready);
    }
#endif
}

#pragma GCC diagnostic pop
