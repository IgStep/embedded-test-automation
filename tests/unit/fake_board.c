#include <string.h>
#include "fake_board.h"
#include "board.h"

fake_board_t fake;

void fake_reset(void)
{
    memset(&fake, 0, sizeof(fake));   /* set every byte of the struct to 0 */
}

void fake_clear_tx(void)
{
    fake.tx[0] = '\0';                /* empty C string */
}

/* --- the board.h functions, fake versions --------------------------- */

uint32_t board_millis(void)           { return fake.now_ms; }
void     board_set_leds(uint8_t mask) { fake.leds = mask; }
void     board_toggle_heartbeat(void) { fake.heartbeat_toggles++; }
bool     board_button_raw(void)       { return fake.button; }

int board_uart_getc(void)
{
    if (fake.rx == NULL || *fake.rx == '\0') {
        return -1;
    }
    return (unsigned char)*fake.rx++;  /* give one char, move pointer on */
}

void board_uart_puts(const char *s)
{
    /* strncat appends, but never past the end of tx[] */
    strncat(fake.tx, s, sizeof(fake.tx) - strlen(fake.tx) - 1);
}