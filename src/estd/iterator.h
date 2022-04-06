#pragma once

#include "internal/iterator_standalone.h"
#include "istream.h"



// TODO: Might need a specialization for our accessor-related things. we'll see
namespace estd {
namespace experimental {

template<class TStreambuf>
class istreambuf_iterator
{
public:
    typedef typename TStreambuf::char_type char_type;
    typedef typename TStreambuf::traits_type traits_type;

    typedef TStreambuf streambuf_type;
private:

    streambuf_type* const rdbuf;

public:
    istreambuf_iterator() :
        rdbuf(NULLPTR)
    {
    }

    template <class TIstreamBase>
    istreambuf_iterator(estd::internal::basic_istream<TStreambuf, TIstreamBase>& is) :
        rdbuf(is.rdbuf())
    {
    }
};

// Similar to boost's version, but we don't use a functor (maybe we should?)
// https://www.boost.org/doc/libs/1_67_0/libs/iterator/doc/html/iterator/specialized/filter.html
template <class TPredicate, class TBaseIterator>
class filter_iterator
{
    TPredicate predicate;
    TBaseIterator baseIterator;

    typedef filter_iterator iterator;
    typedef typename iterator_traits<TBaseIterator>::value_type value_type;
    typedef typename iterator_traits<TBaseIterator>::reference reference;

public:
    filter_iterator(TPredicate predicate, TBaseIterator baseIterator) :
        predicate(predicate),
        baseIterator(baseIterator)
    {

    }

    // prefix version
    iterator& operator++()
    {
        ++baseIterator;

        return *this;
    }

    // NOTE: This is kind of a bummer we have to carry TPredicate around too
    // postfix version
    iterator operator++(int)
    {
        iterator temp(*this);
        operator++();
        return temp;
    }

    value_type operator*()
    {
        return predicate(*baseIterator);
    }
};

template <class TPredicate, class TIterator>
filter_iterator<TPredicate, TIterator> make_filter_iterator
    (TPredicate predicate, TIterator it)
{
    return filter_iterator<TPredicate, TIterator>(predicate, it);
}

}
}
