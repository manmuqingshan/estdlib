#pragma once

#include "../type_traits.h"
#include "locale.h"
#include "../cstdint.h"
#include "iosfwd.h"

#include "feature/ios.h"

namespace estd {

class ios_base
{
public:
    typedef uint8_t fmtflags;

    // NOTE: Spec appears to conflict with itself.
    // https://en.cppreference.com/w/cpp/io/ios_base/fmtflags suggests we have carte blanche
    // do make these any values we wish, but
    // https://en.cppreference.com/w/cpp/locale/num_get/get strongly implies that 'dec'
    // is expected to be zero
    static CONSTEXPR fmtflags dec = 0x01;
    static CONSTEXPR fmtflags hex = 0x02;
#if FEATURE_ESTD_OSTREAM_OCTAL
    static CONSTEXPR fmtflags oct = 0x03;
#endif
    static CONSTEXPR fmtflags basefield = dec | hex;
    // Not supported yet
    static CONSTEXPR fmtflags uppercase = 0x04;

    // NOTE: "Has no effect on input"
    // https://en.cppreference.com/w/cpp/io/manip/left
    // But to optimize it into basic_ostream_base would almost definitely break
    // compatibility
    static CONSTEXPR fmtflags left = 0x08;
    static CONSTEXPR fmtflags right = 0x10;
    static CONSTEXPR fmtflags adjustfield = left | right;

    static CONSTEXPR fmtflags boolalpha = 0x20;
    static CONSTEXPR fmtflags unitbuf = 0x40;

    // TODO: Not yet used, right now we are generally hardcoded to skipws = on
    static CONSTEXPR fmtflags skipws = 0x80;


    typedef uint8_t openmode;

    static CONSTEXPR openmode app = 0x01;
    static CONSTEXPR openmode binary = 0x02;
    static CONSTEXPR openmode in = 0x04;
    static CONSTEXPR openmode out = 0x08;

    typedef uint8_t iostate;

    // DEBT: Try making these enum if we can, primarily to auto deduce
    // bit field size

    static CONSTEXPR iostate goodbit = 0x00;
    static CONSTEXPR iostate badbit = 0x01;
    static CONSTEXPR iostate failbit = 0x02;
    static CONSTEXPR iostate eofbit = 0x04;

    // Non standard, experimental.  Reflects that we are in a wait state
    // to see if there is any more data.  Maps to 'showmanyc' value of 0.
    static CONSTEXPR iostate nodatabit = 0x08;

    typedef uint8_t seekdir;

    static CONSTEXPR seekdir beg = 0x00;
    static CONSTEXPR seekdir end = 0x01;
    static CONSTEXPR seekdir cur = 0x02;

private:
    struct state
    {
#if FEATURE_ESTD_AGGRESSIVE_BITFIELD
        fmtflags fmtfl_ : 8;
        // DEBT: Experimental nodatabit doesn't fit here
        iostate iostate_ : 4;
        // Width applies to istream *and* ostream
        unsigned width_ : 4;

        constexpr state() :
            fmtfl_{dec | right},
            iostate_{goodbit},
            width_{0}
        {}

#else
        unsigned width_ : 16;
        fmtflags fmtfl_ : 8;
        iostate iostate_ : 8;

        constexpr state() :
            width_{0},
            fmtfl_{dec | right},
            iostate_{goodbit}
        {}

#endif

    }   state_;

protected:
    static CONSTEXPR openmode _openmode_null = 0; // proprietary, default of 'text'

    // remove state, not official call
    // UNTESTED
    void unsetstate(iostate state)
    {
        state_.iostate_ &= ~state;
    }

public:
    ios_base() = default;

    constexpr streamsize width() const
    {
        return state_.width_;
    }

    streamsize width(streamsize new_width)
    {
        streamsize old_width = width();
        state_.width_ = new_width;
        return old_width;
    }


    fmtflags setf(fmtflags flags)
    { fmtflags prior = state_.fmtfl_; state_.fmtfl_ |= flags; return prior; }

    fmtflags setf(fmtflags flags, fmtflags mask)
    {
        fmtflags prior = state_.fmtfl_;
        state_.fmtfl_ &= ~mask;
        state_.fmtfl_ |= flags;
        return prior;
    }

    fmtflags unsetf(fmtflags flags)
    { fmtflags prior = state_.fmtfl_; state_.fmtfl_ &= ~flags; return prior; }

    ESTD_CPP_CONSTEXPR_RET fmtflags flags() const
    { return state_.fmtfl_; }

    fmtflags flags(fmtflags fmtfl)
    { fmtflags prior = state_.fmtfl_; state_.fmtfl_ = fmtfl; return prior; }

    ESTD_CPP_CONSTEXPR_RET iostate rdstate() const
    { return state_.iostate_; }

    void clear(iostate state = goodbit)
    { state_.iostate_ = state; }

    void setstate(iostate state)
    {
        state_.iostate_ |= state;
    }

    ESTD_CPP_CONSTEXPR_RET bool good() const
    { return rdstate() == goodbit; }

    ESTD_CPP_CONSTEXPR_RET bool bad() const
    { return rdstate() & badbit; }

    ESTD_CPP_CONSTEXPR_RET bool fail() const
    { return rdstate() & failbit || rdstate() & badbit; }

    ESTD_CPP_CONSTEXPR_RET bool eof() const
    { return rdstate() & eofbit; }

protected:
    // internal call which we may make a layer0 version for optimization
    ESTD_CPP_CONSTEXPR_RET bool is_unitbuf_set() const { return state_.fmtfl_ & unitbuf; }
};

// NOTE: these are not heeded quite yet
inline ios_base& unitbuf(ios_base& s)
{
    s.setf(ios_base::unitbuf);
    return s;
}

inline ios_base& nounitbuf(ios_base& s)
{
    s.unsetf(ios_base::unitbuf);
    return s;
}


namespace internal {

class basic_ostream_base
{
protected:
#if FEATURE_ESTD_OSTREAM_SETW
    // NOTE: Deviates from spec - std wants this in ios_base as part of setf
    // DEBT: Super clumsy, may want additional layer of wrappers for enum class
    // NOTE: Due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=51242#c31 below,
    // not using scoped enum at all in this case
#if defined(FEATURE_CPP_ENUM_CLASS) && !(__GNUC__ < 10)
    enum class positioning_type
#else
    struct positioning_type { enum values
#endif
    {
        left = 0,
        right,
        internal
    };
#if defined(FEATURE_CPP_ENUM_CLASS) && !(__GNUC__ < 10)
    using positioning = positioning_type;
#else
    };

    typedef typename positioning_type::values positioning;
#endif

    struct ostream_internal
    {
        // NOTE: Hitting compiler warning bug
        // https://stackoverflow.com/questions/36005063/gcc-suppress-warning-too-small-to-hold-all-values-of
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=51242#c31
        positioning alignment : 2;  // DEBT: Unused

        // DEBT: Presumes ASCII, 7-bit.  Unicode and friends are out
        char fillchar : 6;          // + 32 (from ' ' to '`' ASCII)
        //bool showbase : 1;          // DEBT: Unused

        ESTD_CPP_CONSTEXPR_RET ostream_internal() :
            alignment(positioning_type::left),
            fillchar(0) // equivalent to space ' '
        {
        }

    }   ostream_;

#endif

};

}

}
