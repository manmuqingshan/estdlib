#include "unit-test.h"

#if defined (FEATURE_CPP_VARIADIC)
#include <estd/functional.h>

static void test_inline_function()
{
    int counter = 0;

    auto f = estd::experimental::function<void()>::make_inline2([&](){++counter;});

    // ESP32 This fails for 2/3 conditions.  For:
    // concept_fnptr1 - hard crash
    // concept_fnptr2 - does not appear to execute
    // concept_virtual - works OK
    f();

    TEST_ASSERT_EQUAL(1, counter);
}
#else
static void test_inline_function()
{
    
}
#endif

#ifdef ESP_IDF_TESTING
TEST_CASE("runtime function tests", "[functional]")
#else
void test_functional()
#endif
{
	RUN_TEST(test_inline_function);
}

