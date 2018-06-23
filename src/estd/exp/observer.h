#pragma once

#include "../internal/platform.h"

// inspired by https://github.com/ETLCPP/etl/blob/master/include/etl/observer.h

namespace estd { namespace experimental {

namespace internal {

template <class TContainer>
class subject
{
    typedef typename TContainer::value_type observer_type;
public:

};

#ifdef FEATURE_CPP_VARIADIC
template <class ...>
class stateless_subject
{

};



template <class TObserver>
class stateless_subject<TObserver>
{
public:
    template <class TNotification>
    static void notify(const TNotification& n)
    {
        TObserver::on_notify(n);
    }
};

template <class TObserver, class ...TObservers>
class stateless_subject<TObserver, TObservers...>
{
    typedef stateless_subject<TObservers...> base_t;

public:
    template <class TNotification>
    static void notify(const TNotification& n)
    {
        base_t::notify(n);
        TObserver::on_notify(n);
    }
};
#endif


}

#ifdef FEATURE_CPP_VARIADIC
namespace layer0 {

    /*
template <class TObserver, TObserver& o, TObserver&..._>
class subject;

template <class TObserver, TObserver& observer>
class subject
{
public:
    template <class TNotification>
    static void notify(const TNotification& n)
    {
        observer.on_notify(n);
    }
};

template <class TObserver, TObserver& observer, TObserver&...observers >
class subject
{
    typedef subject<TObserver, observers...> base_t;

    template <class TNotification>
    static void _notify(const TNotification& n)
    {
        base_t::_notify(n);

        observer.on_notify(n);
    }

public:
    template <class TNotification>
    static void notify(const TNotification& n)
    {
        _notify<TNotification>(n);
    }
};
 */
}
#endif

}}