#pragma once

#include "semaphore.h"

namespace estd {

namespace freertos {

namespace internal {

class mutex_base : public semaphore_base
{
protected:
    mutex_base(SemaphoreHandle_t s) :
        semaphore_base(s) {}

public:
    void lock()
    {
        s.take(portMAX_DELAY);
    }

    bool try_lock()
    {
        return s.take(0);
    }

    bool unlock()
    {
        return s.give() == pdTRUE;
    }

};

// UNTESTED
// TODO: Add an ESTD_FLAG to denote capabilities
class isr_mutex_base : public semaphore_base
{
protected:
    constexpr isr_mutex_base(SemaphoreHandle_t s) :
        semaphore_base(s) {}

public:
    // No blocking flavor here
    //void lock()

    bool try_lock() const
    {
        return s.take_from_isr() == pdTRUE;
    }

    bool unlock() const
    {
        return s.give_from_isr() == pdTRUE;
    }

};


class recursive_mutex_base : public semaphore_base
{
public:
    recursive_mutex_base(SemaphoreHandle_t s) :
        semaphore_base(s) {}

    void lock()
    {
        s.take_recursive(portMAX_DELAY);
    }

    bool try_lock()
    {
        return s.take_recursive(0);
    }

    bool unlock()
    {
        return s.give_recursive() == pdTRUE;
    }
};

}

template <>
class mutex<false> : public internal::mutex_base
{
public:
    mutex(bool binary = false) :
        internal::mutex_base(binary ?
            xSemaphoreCreateBinary() :
            xSemaphoreCreateMutex())
    {
    }
};


#if configSUPPORT_STATIC_ALLOCATION
template <>
class mutex<true> : public internal::mutex_base
{
    typedef internal::mutex_base base_type;

protected:
    StaticSemaphore_t storage;

public:
    mutex(bool binary = false) :
        base_type(binary ?
            wrapped::create(binary_tag(), &storage) : 
            wrapped::create(mutex_tag(), &storage))
    {
    }
};
#endif

template <bool static_allocated>
class timed_mutex : public mutex<static_allocated>
{
    typedef mutex<static_allocated> base_type;

public:
    timed_mutex(bool binary = false) : base_type(binary) {}
    
    template< class Rep, class Period >
    bool try_lock_for(const estd::chrono::duration<Rep,Period>& timeout_duration)
    {
        // This should convert whatever incoming time format duration into our steady_clock
        // duration, which is very specifically freertos-tick bound
        // if this works well, use duration_cast instead and directly
        // on xSemaphoreTake
        estd::chrono::freertos_clock::duration d = timeout_duration;

        // get number of ticks
        return xSemaphoreTake(base_type::s, d.count());
    }

    template<class Clock, class Duration>
    bool try_lock_until(const estd::chrono::time_point<Clock,Duration>& timeout_time)
    {
        return xSemaphoreTake(base_type::s, timeout_time - chrono::freertos_clock::now());
    }
};

template <>
class recursive_mutex<false> : public internal::recursive_mutex_base
{
public:
    recursive_mutex() :
        internal::recursive_mutex_base(xSemaphoreCreateRecursiveMutex())
    {}
};


#if configSUPPORT_STATIC_ALLOCATION
template <>
class recursive_mutex<true> : public internal::recursive_mutex_base
{
    StaticSemaphore_t storage;

public:
    recursive_mutex() : internal::recursive_mutex_base(
        xSemaphoreCreateRecursiveMutexStatic(&storage))
    {}
};
#endif

}

#if FEATURE_ESTD_FREERTOS_THREAD
typedef freertos::mutex<false> mutex;
typedef freertos::timed_mutex<false> timed_mutex;
typedef freertos::recursive_mutex<false> recursive_mutex;
#endif

}
