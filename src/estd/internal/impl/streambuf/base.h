#pragma once

#include "fwd.h"

#include "../../feature/streambuf.h"
#include "../../../policy/rfc.h"

namespace estd { namespace internal { namespace impl {

/// @brief contains base noop-ish implementation, suitable for hiding (think override,
/// but without the virtual since we're all templated)
/// Derived out classes MUST implement:
/// - xsputn
/// Derived in classes MUST implement:
/// - xsgetn
/// - xsgetc
/// Additionally, they MAY implement:
/// - sputc
/// - gbump
template <ESTD_CPP_CONCEPT(concepts::v1::CharTraits) Traits, class Signal>
struct streambuf_base
{
#if FEATURE_ESTD_STREAMBUF_TRAITS
    // 29JUN25 MB - NOTE: This has been disabled for a while but was coming along before I disabled it.
    // Consider strongly changing blocking, ncopy etc. to rfc category.  Alternatively, a freestanding
    // 'policy' to do those rfc indications could be interesting and avoid reinterpreting meaning of 'traits'
    struct traits_type : Traits
    {
        // Since we deal with specialization, sometimes folks need the plain and pure char_traits
        using char_traits = Traits;

        // estd streambufs are presumed non blocking.  Once in a while however underlying system
        // only provides a blocking interface (i.e. Arduino), so account for that
        constexpr static bool blocking = false;

        // EXPERIMENTAL - denotes an appended async bool flag on xsgetn and xsputn.  For both, when true
        // specified data buffer must be non volatile for duration of transaction.  Underlying transport
        // signals (somehow, TBD) that NV status is released.
        constexpr static bool nocopy = false;

        // EXPERIMENTAL - impl to place into consuming stream to interact with whatever specialized
        // signaling mechanism is present
        // TODO: Consider flowing this in externally.  Either additional template parameter,
        // or upgrade 'Traits' expectation to include this.  Note that the latter has an issue
        // that some parties still specialization on std traits.  So be ginger when trying that out.
        // Also it presents an architectural issue, since streambuf itself wants to define the
        // traits it has rather than an external party.... except for signal, partially.  Signal
        // is a shared scheme between system and streambuf's likely-bespoke signaler (i.e.
        // netconn callbacks)
        struct signal
        {
            static constexpr bool set_cts() { return {}; }
            static constexpr bool set_dtr() { return {}; }
        };
    };

    template <class Signal2>
    static constexpr bool add_signal(Signal2*) { return{}; }
    template <class Signal2>
    static constexpr bool del_signal(Signal2*) { return{}; }

#else
    typedef Traits traits_type;
#endif
    typedef typename traits_type::char_type char_type;
    typedef typename traits_type::int_type int_type;
    typedef typename traits_type::pos_type pos_type;
    typedef typename traits_type::off_type off_type;

#if FEATURE_ESTD_STREAMBUF_POLICY
    // 21JUL25 MB EXPERIMENTAL, displaces FEATURE_ESTD_STREAMBUF_TRAITS
    struct policy
    {
        using rfc = internal::rfc::rfc2119;

        static constexpr bool blocking = false;

        struct use
        {
            static constexpr rfc gptr = rfc::must_not;
            static constexpr rfc pptr = rfc::must_not;
            static constexpr rfc seekoff = rfc::must_not;
            static constexpr rfc seekpos = rfc::must_not;
        };
    };
#endif

protected:
    static ESTD_CPP_CONSTEVAL int sync() { return 0; }

    // 0 = "unsure if there are characters available in the associated sequence".
    static ESTD_CPP_CONSTEVAL streamsize showmanyc() { return 0; }

    static ESTD_CPP_CONSTEVAL pos_type seekpos(pos_type, ios_base::openmode)
    {
        return pos_type(off_type(-1));
    };

    static ESTD_CPP_CONSTEVAL pos_type seekoff(off_type, ios_base::seekdir, ios_base::openmode)
    {
        return pos_type(off_type(-1));
    };

    inline static int_type uflow() { return traits_type::eof(); }

    inline static int_type pbackfail(int_type c = traits_type::eof())
    {
        return traits_type::eof();
    }

    inline static int_type overflow(int_type ch = traits_type::eof())
    {
        return traits_type::eof();
    }

    inline static int_type underflow() { return traits_type::eof(); }

    // Non-standard API feeder for showmanyc/in_avail
    // Only reports on available or not, does not take any guesses
    // as to what might come if the buffer is filled
    inline static int_type xin_avail() { return 0; }

    // Helper to produce showmanyc-style return values from regular
    // in avail style values.  Remember "0" for showmanyc means
    // unknown character availabity
    inline static streamsize showmanyc(int_type avail, bool eof = true)
    {
        if(eof)
            // eof flag tells us whether there's any ambiguity about actually being end of buffer
            return avail > 0 ? avail : -1;
        else
            return avail;
    }
};


// Expect most native stream implementations can use this helper base impl
template <class TChar, class TStream, class TCharTraits >
struct native_streambuf_base : streambuf_base<TCharTraits>
{
    using base_type = streambuf_base<TCharTraits>;
    using typename base_type::traits_type;

    // NOTE: we'll need to revisit this if we want a proper pointer in here
    typedef typename estd::remove_reference<TStream>::type stream_type;
    typedef TChar char_type;
    typedef typename traits_type::int_type int_type;

protected:
    TStream stream;

    native_streambuf_base(stream_type& stream) : stream(stream)
    {}

#if __cpp_rvalue_reference
    native_streambuf_base(stream_type&& move_from) : stream(std::move(move_from)) {}
#endif
};


// For streambufs which wrap other streambufs, this is a convenient helper
// DEBT: Really should be worked out with instance_provider/instance_evaporator
// and friends
template <class Streambuf>
class wrapped_streambuf_base :
    public streambuf_base<typename remove_reference<Streambuf>::type::traits_type>
{
protected:
    typedef typename remove_reference<Streambuf>::type streambuf_type;

    // TODO: Make this an is-a not a has-a so we can do both istreambuf and ostreambuf
    Streambuf streambuf_;

    ESTD_CPP_FORWARDING_CTOR_MEMBER(wrapped_streambuf_base, streambuf_)

public:
    // Goofy but sensible rdbuf() resulting in possible rdbuf()->rdbuf() calls
    streambuf_type& rdbuf() { return streambuf_; }
    const streambuf_type& rdbuf() const { return streambuf_; }
};

}}}
