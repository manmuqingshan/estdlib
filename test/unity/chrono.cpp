/**
 * @file
 */

#include "unit-test.h"

#include <estd/chrono.h>
#include <estd/thread.h>

#if ESP_PLATFORM
#include <esp_log.h>
#endif

#undef min
#undef max

static void test_chrono_convert()
{
    estd::chrono::minutes m(2);

    estd::chrono::seconds s = estd::chrono::duration_cast<estd::chrono::seconds>(m);

    TEST_ASSERT_EQUAL(120, s.count());
}

static void test_chrono_subtract()
{
    estd::chrono::seconds s(10);
    estd::chrono::minutes m(2);

    estd::common_type<estd::chrono::seconds,estd::chrono::minutes>::type d;

    d = m - s;

    TEST_ASSERT_EQUAL_INT(110, d.count());

    estd::chrono::milliseconds ms(100);

    ms = d - ms;

    TEST_ASSERT_EQUAL_INT(110000 - 100,ms.count());
}

template <class Clock>
void test_clock()
{
#if ESP_PLATFORM
    static constexpr const char* TAG = "test_clock";
#endif

    typedef Clock clock_type;
    typedef typename clock_type::time_point time_point;
    //typedef typename time_point::duration duration;

    clock_type c;

    //CONSTEXPR time_point min = time_point::min();
    // Implicit zero ought to be enough
    CONSTEXPR time_point min;
    const time_point now = c.now();

#if ESP_PLATFORM
    ESP_LOGV(TAG, "min.count()=%ld, now.count()=%ld",
        (long)min.time_since_epoch().count(),
        (long)now.time_since_epoch().count());
#endif

    // 09DEC24 MB Fun fact: ESP32C6 comes up so fast, FreeRTOS tick is 0 == 0
    TEST_ASSERT_GREATER_OR_EQUAL(
        min.time_since_epoch().count(),
        now.time_since_epoch().count());
    //TEST_ASSERT(n > min);
}


// NOTE: Right now we aggressively alias our own _clock into steady_clock
// so this test at the moment overlaps with others, except for robust environments
// like esp-idf which have a proper std::chrono::steady_clock implementation
#if FEATURE_STD_CHRONO_CLOCK
static void test_std_steady_clock()
{
    test_clock<std::chrono::steady_clock>();
}

static void test_std_system_clock()
{
    test_clock<std::chrono::system_clock>();
}

static void test_estd_steady_clock()
{
    test_clock<estd::chrono::steady_clock>();
}

static void test_estd_system_clock()
{
    // DEBT: Nobody seems to have this, but it feels out of alignment
    // with estd::chrono::steady_clock presence
    //test_clock<estd::chrono::system_clock>();
}

#endif

#ifdef ESTD_OS_FREERTOS
static void test_freertos_clock()
{
    using namespace estd;

    using clock = chrono::freertos_clock;

    test_clock<clock>();

    this_thread::sleep_until(clock::now() + chrono::milliseconds(0));
}
#endif

#ifdef ESTD_SDK_IDF
static void test_esp_idf_clock()
{
    test_clock<estd::chrono::esp_clock>();
}
#endif


#ifdef FEATURE_CPP_USER_LITERAL
static void test_literals()
{
    using namespace estd::literals::chrono_literals;

    estd::chrono::milliseconds s = 10s + 5s;

    s += 5s;

    TEST_ASSERT_EQUAL(20000, s.count());
}
#endif

#if FEATURE_STD_CHRONO_CLOCK && ESP_PLATFORM
void setUp_chrono()
{
    ESP_LOGD("setUp_chrono", "entry");

    struct timeval tv = {0, 0};
    TEST_ASSERT_NOT_EQUAL(-1, settimeofday(&tv, 0));
}
#endif

#ifdef ESP_IDF_TESTING
TEST_CASE("chrono tests", "[chrono]")
#else
void test_chrono()
    //UnitySetTestFile(__FILE__);
#endif
{
    RUN_TEST(test_chrono_convert);
    RUN_TEST(test_chrono_subtract);
#if FEATURE_STD_CHRONO_CLOCK
    RUN_TEST(test_std_steady_clock);
    RUN_TEST(test_std_system_clock);
    RUN_TEST(test_estd_steady_clock);
    RUN_TEST(test_estd_system_clock);
#endif
#ifdef ESTD_OS_FREERTOS
    RUN_TEST(test_freertos_clock);
#endif
#ifdef ESTD_SDK_IDF
    RUN_TEST(test_esp_idf_clock);
#endif
#ifdef FEATURE_CPP_USER_LITERAL
    RUN_TEST(test_literals);
#endif
}

