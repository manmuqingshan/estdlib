#include "unit-test.h"

#include <estd/queue.h>
#include <estd/mutex.h>

using namespace estd;

CONSTEXPR int test_value_1 = 5;
CONSTEXPR int test_value_2 = 10;

struct TestStruct
{
    int value;

    TestStruct(int value) : value(value) {}

    // DEBT: Necessary to participate in layer1::queue due to incomplete implementation of
    // whatever that value-holder type was
    TestStruct() {}
};

static void test_queue_1()
{
    layer1::queue<int, 10> q;

    q.push(test_value_1);
    q.push(test_value_2);

    int value = q.front();

    TEST_ASSERT_EQUAL_INT(test_value_1, value);

    q.pop();
    q.pop();

    TEST_ASSERT_EQUAL_INT(0, q.size());
}


static void test_queue_2()
{
    layer1::queue<TestStruct, 10> q;

    // NOTE: Interesting that VisualDSP permits this - taking a const reference to a temporary.  I'm glad
    // it does, but it is a potentially dangerous operation (fortunately not in this case)
    q.push(TestStruct(test_value_1));
}

#ifdef ESP_IDF_TESTING
TEST_CASE("queue tests", "[queue]")
#else
void test_queue()
#endif
{
    RUN_TEST(test_queue_1);
    RUN_TEST(test_queue_2);
}
