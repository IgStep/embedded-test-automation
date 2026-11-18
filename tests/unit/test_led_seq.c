/*
 * Unit tests for led_seq.c (pure logic, no board at all).
 * Framework: Unity (ThrowTheSwitch.org), the most used C test framework
 * in embedded. Each test is a void function; RUN_TEST calls it.
 * setUp() runs before every test, tearDown() after.
 * Test IDs (UT-SEQ-xx) map to requirements in docs/TEST_PLAN.md.
 */
#include "unity.h"
#include "led_seq.h"

static led_seq_t s;     /* fresh for every test, see setUp */

void setUp(void)    { led_seq_init(&s, 0); }
void tearDown(void) { }

/* UT-SEQ-01  REQ-01 */
static void test_starts_on_first_led(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, s.index);
    TEST_ASSERT_EQUAL_HEX8(0x01, led_seq_mask(&s));
    TEST_ASSERT_EQUAL_UINT32(LED_PERIOD_DEFAULT, s.period_ms);
}

/* UT-SEQ-02  REQ-02 */
static void test_does_not_step_before_period(void)
{
    TEST_ASSERT_FALSE(led_seq_update(&s, 999));
    TEST_ASSERT_EQUAL_UINT8(0, s.index);
}

/* UT-SEQ-03  REQ-02 (boundary value: exactly 1000 ms) */
static void test_steps_exactly_at_period(void)
{
    TEST_ASSERT_TRUE(led_seq_update(&s, 1000));
    TEST_ASSERT_EQUAL_UINT8(1, s.index);
    TEST_ASSERT_EQUAL_HEX8(0x02, led_seq_mask(&s));
}

/* UT-SEQ-04  REQ-01 */
static void test_wraps_from_last_to_first(void)
{
    for (uint32_t t = 1000; t <= 7000; t += 1000) {
        led_seq_update(&s, t);
    }
    TEST_ASSERT_EQUAL_UINT8(0, s.index);   /* 7 steps: 0->1->...->6->0 */
}

/* UT-SEQ-05  REQ-03 */
static void test_exactly_one_led_is_on(void)
{
    for (uint32_t i = 0; i < 20; i++) {
        uint8_t m = led_seq_mask(&s);
        TEST_ASSERT_TRUE(m != 0 && (m & (m - 1)) == 0);   /* power of two = 1 bit */
        TEST_ASSERT_TRUE(m < (1u << LED_COUNT));          /* only PC0..PC6       */
        led_seq_update(&s, (i + 1) * 1000);
    }
}

/* UT-SEQ-06  REQ-04 */
static void test_reverse_goes_from_first_to_last(void)
{
    led_seq_reverse(&s);
    led_seq_update(&s, 1000);
    TEST_ASSERT_EQUAL_UINT8(6, s.index);
    led_seq_update(&s, 2000);
    TEST_ASSERT_EQUAL_UINT8(5, s.index);
    led_seq_reverse(&s);
    led_seq_update(&s, 3000);
    TEST_ASSERT_EQUAL_UINT8(6, s.index);
}

/* UT-SEQ-07  REQ-02 (no drift when the loop is late) */
static void test_late_call_keeps_rhythm(void)
{
    led_seq_update(&s, 1003);                   /* 3 ms late */
    TEST_ASSERT_FALSE(led_seq_update(&s, 1999));
    TEST_ASSERT_TRUE(led_seq_update(&s, 2000)); /* still on the 1000 ms grid */
}

/* UT-SEQ-08  REQ-06 (boundary values) */
static void test_speed_limits(void)
{
    led_seq_faster(&s);
    TEST_ASSERT_EQUAL_UINT32(500, s.period_ms);
    for (int i = 0; i < 10; i++) led_seq_faster(&s);
    /* Literal 125, not LED_PERIOD_MIN: if someone changes the constant,
       the requirement (125 ms) should make this test fail. */
    TEST_ASSERT_EQUAL_UINT32(125, s.period_ms);
    for (int i = 0; i < 10; i++) led_seq_slower(&s);
    TEST_ASSERT_EQUAL_UINT32(2000, s.period_ms);
}

/* UT-SEQ-09  REQ-07 */
static void test_pause_stops_and_resume_gives_full_period(void)
{
    led_seq_toggle_pause(&s, 500);
    TEST_ASSERT_FALSE(led_seq_update(&s, 5000));
    TEST_ASSERT_EQUAL_UINT8(0, s.index);
    led_seq_toggle_pause(&s, 5000);             /* resume */
    TEST_ASSERT_FALSE(led_seq_update(&s, 5999));
    TEST_ASSERT_TRUE(led_seq_update(&s, 6000));
    TEST_ASSERT_EQUAL_UINT8(1, s.index);
}

/* UT-SEQ-10  REQ-02 (ms counter wraps after 49.7 days) */
static void test_survives_millis_wraparound(void)
{
    led_seq_init(&s, 0xFFFFFF00u);              /* 256 ms before wrap */
    TEST_ASSERT_FALSE(led_seq_update(&s, 0x00000100u));  /* 512 ms later  */
    TEST_ASSERT_TRUE(led_seq_update(&s, 0x000002E8u));   /* 1000 ms later */
    TEST_ASSERT_EQUAL_UINT8(1, s.index);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_starts_on_first_led);
    RUN_TEST(test_does_not_step_before_period);
    RUN_TEST(test_steps_exactly_at_period);
    RUN_TEST(test_wraps_from_last_to_first);
    RUN_TEST(test_exactly_one_led_is_on);
    RUN_TEST(test_reverse_goes_from_first_to_last);
    RUN_TEST(test_late_call_keeps_rhythm);
    RUN_TEST(test_speed_limits);
    RUN_TEST(test_pause_stops_and_resume_gives_full_period);
    RUN_TEST(test_survives_millis_wraparound);
    return UNITY_END();    /* returns number of failures: 0 = pass */
}
