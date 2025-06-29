#pragma once

#include "fwd.h"
#include "dynamic_array.hpp"

namespace estd {

// NOTE: Works well, just needs more testing (and hopefully elevation of experimental::num_get
// to non-experimental) before elevating to API level
template <class Streambuf, class Base, class T>
typename enable_if<
    is_arithmetic<T>::value && is_same<T, char>::value == false,
    detail::basic_istream<Streambuf, Base>&>::type
operator >>(detail::basic_istream<Streambuf, Base>& in, T& value)
{
    // NOTE:
    // "Except where stated otherwise, calling this function does not alter the
    //  value returned by member gcount." [2]
    // Since gcount is mentioned nowhere else on the page, we don't update gcount.
    // A very specific gcount update is mentioned for scenario #11 in [1], which
    // is outside the scope of this method.
    typedef detail::basic_istream<Streambuf, Base> istream_type;
    typedef typename istream_type::streambuf_type streambuf_type;
    typedef typename istream_type::traits_type traits_type;
    typedef typename traits_type::char_type char_type;
    typedef estd::istreambuf_iterator<streambuf_type> iterator_type;

    in >> ws;

    iterator_type it(in.rdbuf()), end;
    ios_base::iostate err = in.rdstate();

    num_get<char_type, iterator_type> n;

    n.get(it, end, in, err, value);

    // DEBT: Think I'd prefer a friend operation and pass in 'in' directly on n.get
    // DEBT: It's unclear whether we overwrite or OR state here, but I am betting it's OR and that's
    // what code currently does.  When we find conclusive answer, document it
    in.setstate(err);

    return in;
}

// DEBT: Mate this instead to inherent char_type.  Also, spec calls for unsigned char
// https://en.cppreference.com/w/cpp/io/basic_istream/operator_gtgt2
template <class Streambuf, class Base>
detail::basic_istream<Streambuf, Base>& operator >>(
    detail::basic_istream<Streambuf, Base>& in,
    char& value)
{
    using stream_type = detail::basic_istream<Streambuf, Base>;
    using traits = typename stream_type::traits_type;

    in >> ws;

    const typename traits::int_type c = in.get();

    if(traits::not_eof(c))  value = traits::to_char_type(c);

    return in;
}


// NOTE: This works well - be advised it doesn't filter specifically by string
template <class Streambuf, class Base, class StringImpl>
detail::basic_istream<Streambuf, Base>& operator >>(
    detail::basic_istream<Streambuf, Base>& in,
    internal::dynamic_array<StringImpl>& value)
{
    in >> ws;

    value.clear();

    internal::do_input(in, value);

    return in;
}


// C-style string
template <class Streambuf, class Base, size_t N>
detail::basic_istream<Streambuf, Base>& operator >>(
    detail::basic_istream<Streambuf, Base>& in,
    typename Streambuf::nonconst_char_type (&s)[N])
{
    // layer2::string effectively is a lightweight runtime_array wrapper around
    // a C style string. '0' ensures size = 0 (null terminates beginning)
    layer2::basic_string<typename Streambuf::nonconst_char_type, N> str(s, 0);

    in >> ws;

    internal::do_input(in, str);

    return in;
}

}
