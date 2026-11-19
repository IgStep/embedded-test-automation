/*
 * board.h - the ONLY things the application needs from the hardware.
 *
 * app.c calls these functions and does not know how they work.
 * There are different implementations ("ports") of the same functions:
 *   stm32/board_stm32.c  - real board, uses HAL       (firmware)
 *   tests/fake_board.c   - records calls in memory    (unit tests)
 * The linker picks whichever .c file you compile in. This trick is
 * called a "link seam" and it is the standard way to test embedded C.
 */
#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

uint32_t board_millis(void);              /* ms since start (HAL_GetTick) */
void     board_set_leds(uint8_t mask);    /* bit i -> PCi                 */
void     board_toggle_heartbeat(void);    /* onboard LED LD2 (PA5)        */
bool     board_button_raw(void);          /* true while B1 (PC13) pressed */
int      board_uart_getc(void);           /* next received char, -1 if none */
void     board_uart_puts(const char *s);  /* send a text over USART2      */

#endif /* BOARD_H */
