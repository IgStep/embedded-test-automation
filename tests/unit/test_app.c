/*
 * Tests for app.c, running against fake_board.c instead of real HAL.
 * These are "component" tests: led_seq + button + app together,
 * with only the hardware faked.
 */
#include <string.h>
#include "unity.h"
#include "fake_board.h"
#include "app.h"

void setUp(void)
{
    fake_reset();
    app_init();
}
void tearDown(void) { }

/* Run the main loop once per simulated millisecond until 'until_ms'. */
static void run_until(uint32_t until_ms)
{
    while (fake.now_ms < until_ms) {
        fake.now_ms++;
        app_loop_once();
    }
}

/* UT-APP-01  REQ-01, REQ-08 */
static void test_boot_lights_pc0_and_reports(void)
{
    TEST_ASSERT_EQUAL_HEX8(0x01, fake.leds);
    TEST_ASSERT_EQUAL_STRING("BOOT led_seq 1.0.0\r\n"
                             "STEP 0 DIR + PERIOD 1000 T 0\r\n", fake.tx);
}

/* UT-APP-02  REQ-02, REQ-09 */
static void test_step_toggles_heartbeat_and_prints(void)
{
    fake_clear_tx();
    run_until(999);
    TEST_ASSERT_EQUAL_HEX8(0x01, fake.leds);
    TEST_ASSERT_EQUAL_INT(0, fake.heartbeat_toggles);
    run_until(1000);
    TEST_ASSERT_EQUAL_HEX8(0x02, fake.leds);
    TEST_ASSERT_EQUAL_INT(1, fake.heartbeat_toggles);
    TEST_ASSERT_EQUAL_STRING("STEP 1 DIR + PERIOD 1000 T 1000\r\n", fake.tx);
}

/* UT-APP-03  REQ-01, REQ-02, REQ-09: same behaviour as the original loop */
static void test_two_full_cycles_match_original_program(void)
{
    for (uint32_t k = 1; k <= 14; k++) {
        run_until(k * 1000);
        TEST_ASSERT_EQUAL_HEX8(1u << (k % 7), fake.leds);
    }
    TEST_ASSERT_EQUAL_INT(14, fake.heartbeat_toggles);
}

/* UT-APP-04  REQ-04 */
static void test_uart_r_reverses(void)
{
    fake_clear_tx();
    fake.rx = "r";
    run_until(1000);
    TEST_ASSERT_NOT_NULL(strstr(fake.tx, "OK 0 DIR - PERIOD 1000 T 1\r\n"));
    TEST_ASSERT_EQUAL_HEX8(1u << 6, fake.leds);     /* went 0 -> 6 */
}

/* UT-APP-05  REQ-06, REQ-08 */
static void test_uart_f_and_status(void)
{
    fake_clear_tx();
    fake.rx = "f?";
    run_until(2);
    TEST_ASSERT_EQUAL_STRING("OK 0 DIR + PERIOD 500 T 1\r\n"
                             "OK 0 DIR + PERIOD 500 T 2\r\n", fake.tx);
    run_until(500);                     /* first step 500 ms after boot */
    TEST_ASSERT_EQUAL_HEX8(0x02, fake.leds);
}

/* UT-APP-06  REQ-06 */
static void test_uart_s_slows_down(void)
{
    fake.rx = "s";
    run_until(1999);
    TEST_ASSERT_EQUAL_HEX8(0x01, fake.leds);        /* 2000 ms period now */
    run_until(2000);
    TEST_ASSERT_EQUAL_HEX8(0x02, fake.leds);
}

/* UT-APP-07  REQ-08: negative test */
static void test_uart_unknown_char_is_error_and_enter_ignored(void)
{
    fake_clear_tx();
    fake.rx = "x\r\n";
    run_until(3);
    TEST_ASSERT_EQUAL_STRING("ERR\r\n", fake.tx);
}

/* UT-APP-08  REQ-07 */
static void test_uart_p_pauses(void)
{
    fake.rx = "p";
    run_until(5000);
    TEST_ASSERT_EQUAL_HEX8(0x01, fake.leds);
    TEST_ASSERT_NOT_NULL(strstr(fake.tx, "OK 0 DIR + PERIOD 1000 T 1 PAUSED\r\n"));
}

/* UT-APP-09  REQ-05 */
static void test_button_press_reverses(void)
{
    fake_clear_tx();
    fake.button = true;                 /* press and hold */
    run_until(100);
    fake.button = false;
    TEST_ASSERT_NOT_NULL(strstr(fake.tx, "BTN 0 DIR - PERIOD 1000 T 31\r\n"));
    run_until(1000);
    TEST_ASSERT_EQUAL_HEX8(1u << 6, fake.leds);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_boot_lights_pc0_and_reports);
    RUN_TEST(test_step_toggles_heartbeat_and_prints);
    RUN_TEST(test_two_full_cycles_match_original_program);
    RUN_TEST(test_uart_r_reverses);
    RUN_TEST(test_uart_f_and_status);
    RUN_TEST(test_uart_s_slows_down);
    RUN_TEST(test_uart_unknown_char_is_error_and_enter_ignored);
    RUN_TEST(test_uart_p_pauses);
    RUN_TEST(test_button_press_reverses);
    return UNITY_END();
}