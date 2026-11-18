/*
 * board.h - the ONLY things the application needs from the hardware.
 * (Lesson 4 version: 4 functions. Lesson 5 adds the UART.)
 */
#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

uint32_t board_millis(void);              /* ms since start (HAL_GetTick) */
void     board_set_leds(uint8_t mask);    /* bit i -> PCi                 */
void     board_toggle_heartbeat(void);    /* onboard LED LD2 (PA5)        */
bool     board_button_raw(void);          /* true while B1 (PC13) pressed */

#endif /* BOARD_H */
