/*
 * fake_board.h - a pretend board for tests.
 * Tests set the inputs (time, button, received chars) and read back
 * the outputs (LED mask, heartbeat count, text sent on the UART).
 */
#ifndef FAKE_BOARD_H
#define FAKE_BOARD_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    /* inputs the test controls */
    uint32_t    now_ms;
    bool        button;
    const char *rx;           /* chars "received" by the UART, one per loop */
    /* outputs the test checks */
    uint8_t     leds;
    int         heartbeat_toggles;
    char        tx[4096];     /* everything printed, appended */
} fake_board_t;

extern fake_board_t fake;

void fake_reset(void);
void fake_clear_tx(void);

#endif