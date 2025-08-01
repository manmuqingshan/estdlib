#pragma once

#include "iostream.h"
#include "string.h"
#include "internal/impl/streambuf/string.h"
#include "internal/impl/streambuf/string_view.h"

// TODO: Move non-alias template flavor elsewhere, since it's so bulky
namespace estd {

namespace layer1 {

template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_stringbuf = estd::internal::streambuf <
    estd::internal::impl::basic_stringbuf<layer1::basic_string < Char, N, null_terminated, Traits> > >;

template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_out_stringbuf = estd::internal::streambuf <
    estd::internal::impl::out_stringbuf<layer1::basic_string < Char, N, null_terminated, Traits> > >;

template<size_t N, bool null_terminated = true>
using stringbuf = basic_stringbuf<char, N, null_terminated>;

}


#if __cpp_alias_templates
namespace layer1 {
template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_ostringstream = estd::detail::basic_ostream<layer1::basic_out_stringbuf<Char, N, null_terminated, Traits> >;

template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_istringstream = estd::detail::basic_istream<layer1::basic_stringbuf<Char, N, null_terminated, Traits> >;

template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_stringstream = estd::detail::basic_iostream<layer1::basic_stringbuf<Char, N, null_terminated, Traits> >;

template<size_t N, bool null_terminated = true>
using ostringstream = basic_ostringstream<char, N, null_terminated>;

template<size_t N, bool null_terminated = true>
using istringstream = basic_istringstream<char, N, null_terminated>;

template<size_t N, bool null_terminated = true>
using stringstream = basic_stringstream<char, N, null_terminated>;

}
#endif

namespace experimental {
// 31AUG23 Historically we were unsure if we wanted
// to commit layer1 basic_stringstream to
// only the two major string use cases of null term.  Now, we do - if someone
// has something more exotic in mind, one can always swap in a different variety
// of stringbuf.  Keeping experimental namespace here to not break existing
// code.
// DEBT: phase out stringstream's participation in 'experimental' completely by
// 01OCT23
#ifdef __cpp_alias_templates
template<class TChar, size_t N, bool null_terminated = true, class Traits = estd::char_traits<TChar> >
using basic_ostringstream = estd::detail::basic_ostream<layer1::basic_out_stringbuf<TChar, N, null_terminated, Traits> >;

template<class TChar, size_t N, bool null_terminated = true, class Traits = estd::char_traits<TChar> >
using basic_istringstream = estd::detail::basic_istream<layer1::basic_stringbuf<TChar, N, null_terminated, Traits> >;

template<size_t N, bool null_terminated = true>
using ostringstream = basic_ostringstream<char, N, null_terminated>;

template<size_t N, bool null_terminated = true>
using istringstream = basic_istringstream<char, N, null_terminated>;
#else
template<class TChar, size_t N, bool null_terminated = true, class Traits = estd::char_traits<TChar> >
struct basic_istringstream : estd::detail::basic_istream<layer1::basic_stringbuf<TChar, N, null_terminated, Traits> >
{
    typedef estd::detail::basic_istream<layer1::basic_stringbuf<TChar, N, null_terminated, Traits> > base_type;
    ESTD_CPP_FORWARDING_CTOR(basic_istringstream)
};

template<size_t N, bool null_terminated = true>
struct istringstream : basic_istringstream<char, N, null_terminated>
{
    typedef basic_istringstream<char, N, null_terminated> base_type;
    ESTD_CPP_FORWARDING_CTOR(istringstream)
};

template <size_t N, bool null_terminated = true>
struct ostringstream : estd::detail::basic_ostream<layer1::basic_stringbuf<char, N, null_terminated> >
{
    typedef estd::detail::basic_ostream<layer1::basic_stringbuf<char, N, null_terminated> > base_type;
    ESTD_CPP_FORWARDING_CTOR(ostringstream)
};
#endif

}


namespace layer2 {

#ifdef __cpp_alias_templates
template<class Char, size_t N = 0, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_stringbuf = detail::streambuf <
    internal::impl::basic_stringbuf<
        layer2::basic_string <Char, N, null_terminated, Traits> > >;

using stringbuf = basic_stringbuf<char>;

template <class Char, size_t N = 0, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_istringstream =
    detail::basic_istream<basic_stringbuf<Char, N, null_terminated, Traits>>;
#endif

}


namespace layer3 {

// NOTE: not defaulting null_terminated yet as I'm not sure if we want to lean one way or the
// other for layer3 ostringstream
// also - code compiles but is untested
#ifdef FEATURE_CPP_ALIASTEMPLATE
template<class Char, bool null_terminated, class Traits = estd::char_traits<Char> >
using basic_stringbuf = estd::detail::streambuf <
    estd::internal::impl::basic_stringbuf<
        layer3::basic_string <Char, null_terminated, Traits> > >;

// DEBT: Deviation from norm where 'null terminated' is expected.  Does make sense
// for layer3 since it innately tracks a runtime size anyway.  Still, document our
// thinking here to remove debt
typedef basic_stringbuf<char, false> stringbuf;

template<class Char, bool null_terminated, class Traits = estd::char_traits<Char> >
using basic_ostringstream = estd::detail::basic_ostream<basic_stringbuf<Char, null_terminated, Traits> >;

#endif
}

}
