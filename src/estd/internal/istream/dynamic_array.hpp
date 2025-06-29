#pragma once

#include "../runtime_array.h"

namespace estd { namespace internal {

template <class TStreambuf, class TBase, class TStringImpl>
void do_input(detail::basic_istream<TStreambuf, TBase>& in,
    internal::dynamic_array<TStringImpl>& value)
{
    typedef typename detail::basic_istream<TStreambuf, TBase> istream_type;
    typedef typename estd::remove_reference<TStreambuf>::type impl_type;
    typedef typename impl_type::traits_type traits_type;
    //typedef typename impl_type::char_type char_type;
    typedef typename impl_type::int_type int_type;
    typedef typename istream_type::locale_type locale_type;
    typedef typename istream_type::policy_type policy_type;
    typedef typename istream_type::blocking_type blocking_type;

    locale_type loc = in.getloc();

    for(;;)
    {
        int_type ch = in.peek();

        if(ch == traits_type::eof())
        {
            // If we're non blocking variety, and rdbuf says "unsure if more characters
            // are available", then do our special nodata processing
            if((policy_type::blocking() == false) && in.rdbuf()->in_avail() == 0)
            {
                blocking_type::on_nodata(in, in.rdbuf(), value.size());
            }
            else
            {
                if(value.empty())
                    // "if the function extracts no characters from the input stream." [1]
                    in.setstate(istream_type::failbit);

                in.setstate(istream_type::eofbit);
            }

            break;
        }
        else if(isspace(traits_type::to_char_type(ch), loc)) break;

        // NOTE: += is defined and should have worked
        value.push_back(traits_type::to_char_type(ch));
        //value += (char_type)ch;

        in.get();
    }
}

}}
