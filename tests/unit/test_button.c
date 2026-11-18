/* Unit tests for button.c debouncing. Test IDs map to docs/TEST_PLAN.md. */
#include "unity.h"
#include "button.h"

static button_t b;

void setUp(void)    { button_init(&b, false, 0); }
void tearDown(void) { }

/* Feed the same raw level for every ms in [from, to). Returns event count. */
static int hold(bool raw, uint32_t from, uint32_t to)
{
    int events = 0;
    for (uint32_t t = from; t < to; t++) {
        events += button_update(&b, raw, t);
    }
    return events;
}

/* UT-BTN-01  REQ-05: a 15 ms glitch is shorter than the 30 ms debounce */
static void test_short_glitch_is_ignored(void)
{
    TEST_ASSERT_EQUAL_INT(0, hold(true, 10, 25));
    TEST_ASSERT_EQUAL_INT(0, hold(false, 25, 200));
}

/* UT-BTN-02  REQ-05: contact bounce 1 0 1 0 1, then held -> one event */
static void test_bouncy_press_gives_one_event(void)
{
    int events = 0;
    bool raw[] = { true, false, true, false, true };
    for (uint32_t i = 0; i < 5; i++) {
        events += button_update(&b, raw[i], 10 + i);
    }
    events += hold(true, 15, 200);
    TEST_ASSERT_EQUAL_INT(1, events);
}

/* UT-BTN-03  REQ-05: release is not a press, next press counts again */
static void test_release_gives_no_event_and_next_press_counts(void)
{
    TEST_ASSERT_EQUAL_INT(1, hold(true, 0, 100));
    TEST_ASSERT_EQUAL_INT(0, hold(false, 100, 200));
    TEST_ASSERT_EQUAL_INT(1, hold(true, 200, 300));
}

/* UT-BTN-04  REQ-05: event exactly when held for 30 ms (boundary) */
static void test_event_exactly_at_debounce_time(void)
{
    TEST_ASSERT_EQUAL_INT(0, hold(true, 100, 130));   /* 100..129 */
    TEST_ASSERT_TRUE(button_update(&b, true, 130));   /* 30 ms after 100 */
}

/* UT-BTN-05  REQ-05: button held during reset is not a press */
static void test_held_at_boot_is_not_a_press(void)
{
    button_init(&b, true, 0);
    TEST_ASSERT_EQUAL_INT(0, hold(true, 0, 100));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_short_glitch_is_ignored);
    RUN_TEST(test_bouncy_press_gives_one_event);
    RUN_TEST(test_release_gives_no_event_and_next_press_counts);
    RUN_TEST(test_event_exactly_at_debounce_time);
    RUN_TEST(test_held_at_boot_is_not_a_press);
    return UNITY_END();
}
