/**
 *
 * References
 *
 * 1. https://en.cppreference.com/w/cpp/locale/num_get/get
 */
#pragma once

#include "fwd.h"
#include "cbase.h"
#include "../ios_base.h"
#include "iterated/num_get.h"

#include "../../iosfwd.h"
#include "../streambuf.h"

namespace estd {

template <class TChar, class InputIt = void>
class num_get
{
public:
    typedef TChar char_type;
    typedef InputIt iter_type;

private:

    struct _helper
    {
        // NOTE: iterated::num_get efficiently deals with unsigned as well
        template <unsigned base, class TLocale, class T>
        static iter_type get_signed_integer(iter_type i, iter_type end,
            ios_base& str, ios_base::iostate& err, TLocale l, T& v)
        {
            iterated::num_get<base, char_type, TLocale> n(l);

            for(; i != end; ++i)
            {
                if(n.get(*i, err, v)) return i;
            }

            err |= ios_base::eofbit;
            return i;
        }
    };

    template <class TStreambuf, class TBase>
    struct helper : _helper
    {
        typedef estd::internal::basic_istream<TStreambuf, TBase> istream_type;
        typedef typename istream_type::locale_type locale_type;

        template <unsigned base, class T>
        inline static iter_type get_signed_integer(iter_type i, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            return _helper::template get_signed_integer<base>(
                i, end, str, err, str.getloc(),v);
        }

        template <unsigned base, class T>
        inline static iter_type get_unsigned_integer(iter_type i, iter_type end,
                                            ios_base::iostate& err, istream_type& str, T& v)
        {
            return _helper::template get_signed_integer<base>(
                i, end, str, err, str.getloc(), v);
        }


        static iter_type get_bool(iter_type in, iter_type end,
            ios_base::iostate& err, istream_type& str, bool& v)
        {
            if(str.flags() & ios_base::boolalpha)
            {
                numpunct<char_type, locale_type> np =
                        use_facet<numpunct<char_type> >(str.getloc());

                // tempted to get algorithmically fancy here, but with only two things to
                // compare, brute force makes sense
                estd::layer2::basic_string<const char_type, 0> names[]
                #ifdef FEATURE_CPP_INITIALIZER_LIST
                {
                    np.truename(),
                    np.falsename()
                };
                #else
                ;
                names[0] = np.truename();
                names[1] = np.falsename();
                #endif

                int chosen = estd::internal::chooser::choose(names, in, end);

                if(in == end)
                    err |= ios_base::eofbit;

                if(chosen == -1)
                {
                    v = false;
                    err |= ios_base::failbit;
                }
                else
                    v = chosen == 0;
            }
            else
            {
                unsigned temp;
                in = get_unsigned_integer<2>(in, end, err, str, temp);
                // DEBT: Try to avoid using temporary.
                // No bounds check necessary here, since specifying base 2 already does that
                v = temp == 1;
            }
            return in;
        }

        // TODO: Need to build this out for 0x, 0, etc. support
        template <class T>
        static iter_type get_interpreted(iter_type in, iter_type end,
            istream_type& str, ios_base::iostate& err,
            T& v)
        {
            err |= ios_base::failbit;
            return in;
        }


        // Special thanks to
        // https://stackoverflow.com/questions/9285657/sfinae-differentiation-between-signed-and-unsigned
        // for the hybrid overload/SFINAE approach below

        // types after 'v':
        // 'true_type' = is integer
        // 'false_type' = unsigned
        template <class T>
        static iter_type get(iter_type in, iter_type end,
            istream_type& str, ios_base::iostate& err,
            T& v,
            estd::true_type, estd::false_type)
        {
            const ios_base::fmtflags basefield = str.flags() & estd::ios_base::basefield;

            // TODO: Consider strongly using a switch statement, even though it does
            // spiritually break the spec "the first applicable choice of the following five is selected" [1]

            if(basefield == estd::ios_base::oct)
            {
                return get_unsigned_integer<8>(in, end, err, str, v);
            }
            else if(basefield == estd::ios_base::hex)
            {
                return get_unsigned_integer<16>(in, end, err, str, v);
            }
            else if(basefield == 0)
            {
                return get_interpreted(in, end, str, err, v);
            }
            else  //if(basefield == estd::ios_base::dec)
            {
                return get_unsigned_integer<10>(in, end, err, str, v);
            }
        }

        // types after 'v':
        // 'true_type' = is integer
        // 'true_type' = signed
        template <class T>
        static iter_type get(iter_type in, iter_type end,
            istream_type& str, ios_base::iostate& err,
            T& v,
            estd::true_type, estd::true_type)
        {
            const ios_base::fmtflags basefield = str.flags() & estd::ios_base::basefield;

            if(basefield == estd::ios_base::oct)
            {
                return get_signed_integer<8>(in, end, err, str, v);
            }
            else if(basefield == estd::ios_base::hex)
            {
                // No real negative in hex, so presume caller knows this and passed in a
                // signed type for their own convenience
                return get_unsigned_integer<16>(in, end, err, str, v);
            }
            else if(basefield == 0)
            {
                return get_interpreted(in, end, str, err, v);
            }
            else // if(basefield == estd::ios_base::dec)
            {
                return get_signed_integer<10>(in, end, err, str, v);
            }
        }

        // types after 'v':
        // 'true_type' = is integer
        // 'false_type' = unsigned
        static iter_type get(iter_type in, iter_type end,
            istream_type& str,ios_base::iostate& err,
            bool& v,
            estd::true_type, estd::false_type)
        {
            return get_bool(in, end, err, str, v);
        }
    };

    // TODO: Do a LUT since bounds checking to detect invalid hex chars likely is fastest.  See:
    // https://stackoverflow.com/questions/34365746/whats-the-fastest-way-to-convert-hex-to-integer-in-c
    // Also consider a cut-down one with only maybe 64 characters instead of 128 or 256, because unless
    // we do 256 we have to do bounds checking anyway


public:
    template <typename T, class TStreambuf, class TBase>
    iter_type get(iter_type in, iter_type end,
        estd::internal::basic_istream<TStreambuf, TBase>& str, ios_base::iostate& err, T& v) const
    {
        return helper<TStreambuf, TBase>::get(in, end, str, err, v,
           estd::is_integral<T>(), estd::is_signed<T>());
    }
};

namespace internal {

template <typename TChar, typename TInputIt, class TLocale>
struct use_facet_helper<num_get<TChar, TInputIt>, TLocale>
{
    typedef num_get<TChar, TInputIt> facet_type;

    static facet_type use_facet(TLocale) { return facet_type(); }
};

// Trick to convert a num_char<streambuf_type> into a
// num_char<char_type, istreambuf_iterator<streambuf_type> >
// DEBT: May be better doing this down at num_get level
// DEBT: Really would like this enable_if to take hold so that we could load in other
// types in addition to streambufs.  However, if all we can do is TStreambuf 'default' that
// is acceptable
template <typename TStreambuf
/*, typename estd::enable_if<
        estd::is_base_of<
                estd::internal::streambuf_baseline, TStreambuf>::value,
        TStreambuf>::type* = nullptr> */
        , class TLocale>
struct use_facet_helper<
    num_get<
            /*
        typename estd::enable_if<
            estd::is_base_of<
                estd::internal::streambuf_baseline, TStreambuf>::value,
            TStreambuf>::type, */
        TStreambuf,
        void >,
    TLocale>
    //internal::classic_locale_type>
{
    typedef num_get<
        typename TStreambuf::char_type,
        estd::istreambuf_iterator<TStreambuf> > facet_type;
    //typedef internal::classic_locale_type locale_type;
    typedef TLocale locale_type;

    static facet_type use_facet(locale_type) { return facet_type(); }
};

}

}