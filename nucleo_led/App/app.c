/*
 * app.c - lesson 4 version: running light + button reverses direction.
 */
#include "app.h"
#include "board.h"
#include "led_seq.h"
#include "button.h"

static led_seq_t seq;
static button_t  btn;

void app_init(void)
{
    uint32_t now = board_millis();
    led_seq_init(&seq, now);
    button_init(&btn, board_button_raw(), now);
    board_set_leds(led_seq_mask(&seq));
}

void app_loop_once(void)
{
    uint32_t now = board_millis();

    if (button_update(&btn, board_button_raw(), now)) {   /* one true per press */
        led_seq_reverse(&seq);
    }

    if (led_seq_update(&seq, now)) {
        board_set_leds(led_seq_mask(&seq));
        board_toggle_heartbeat();
    }
}
