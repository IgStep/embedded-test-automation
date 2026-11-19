/*
 * app.c - what the firmware does, written against board.h only.
 *
 * UART output (115200 8N1, lines end with \r\n):
 *   BOOT led_seq 1.0.0                      once after reset
 *   STEP 3 DIR + PERIOD 1000 T 4000         every step (T = ms since boot)
 *   BTN  ...  / OK ...                      after a button press / command
 *   ERR                                     unknown command
 *
 * UART commands (single characters, no Enter needed):
 *   r  reverse direction       f  faster      s  slower
 *   p  pause / resume          ?  print status now
 * The blue button B1 also reverses direction.
 */
#include <stdio.h>      /* snprintf */
#include "app.h"
#include "board.h"
#include "led_seq.h"
#include "button.h"
#include "version.h"

/* "static" at file level = private to this file (other files can't see it). */
static led_seq_t seq;
static button_t  btn;

static void print_status(const char *tag, uint32_t now)
{
    char line[64];
    /* snprintf writes formatted text into line[], never more than
       sizeof(line) bytes, so it can't overflow the buffer.
       %u = unsigned int, %c = one char, %lu = unsigned long. */
    snprintf(line, sizeof(line), "%s %u DIR %c PERIOD %lu T %lu%s\r\n",
             tag,
             (unsigned)seq.index,
             (seq.dir > 0) ? '+' : '-',
             (unsigned long)seq.period_ms,
             (unsigned long)now,
             seq.paused ? " PAUSED" : "");
    board_uart_puts(line);
}

static void handle_command(char c, uint32_t now)
{
    switch (c) {
    case 'r': led_seq_reverse(&seq);           break;
    case 'f': led_seq_faster(&seq);            break;
    case 's': led_seq_slower(&seq);            break;
    case 'p': led_seq_toggle_pause(&seq, now); break;
    case '?':                                  break;
    case '\r': case '\n':                      return; /* ignore Enter */
    default:
        board_uart_puts("ERR\r\n");
        return;
    }
    print_status("OK", now);    /* reply with the new state */
}

void app_init(void)
{
    uint32_t now = board_millis();
    led_seq_init(&seq, now);
    button_init(&btn, board_button_raw(), now);

    board_set_leds(led_seq_mask(&seq));   /* PC0 on right away */
    board_uart_puts("BOOT " FW_NAME " " FW_VERSION "\r\n");  /* "a" "b" = "ab" in C */
    print_status("STEP", now);
}

void app_loop_once(void)
{
    uint32_t now = board_millis();

    if (button_update(&btn, board_button_raw(), now)) {
        led_seq_reverse(&seq);
        print_status("BTN", now);
    }

    int c = board_uart_getc();
    if (c >= 0) {
        handle_command((char)c, now);
    }

    if (led_seq_update(&seq, now)) {
        board_set_leds(led_seq_mask(&seq));
        board_toggle_heartbeat();          /* PA5 toggles every step */
        print_status("STEP", now);
    }
}
