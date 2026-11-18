/*
 * board_stm32.c - the real-hardware version of board.h, using HAL.
 *
 * This is the only application file that includes main.h / HAL.
 * Keep it thin: no decisions here, just "do what you are told".
 * It lives in nucleo_led/App/ next to the portable code.
 */
#include "main.h"      /* HAL + the pin settings CubeMX generated */
#include "board.h"

/* The 7 LED pins of port C, bit i of the mask -> led_pin[i]. */
static const uint16_t led_pin[7] = {
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3,
    GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6
};

uint32_t board_millis(void)
{
    return HAL_GetTick();            /* SysTick counts +1 every ms */
}

void board_set_leds(uint8_t mask)
{
    /* Build two pin masks: pins to turn on, and pins to turn off.
       led_pin[i] is GPIO_PIN_i, so (1 << i) maps 1:1 here. */
    uint16_t on  = 0;
    uint16_t off = 0;
    for (int i = 0; i < 7; i++) {
        if ((mask >> i) & 1u) {      /* (mask >> i) & 1 picks out bit i */
            on  |= led_pin[i];
        } else {
            off |= led_pin[i];
        }
    }
    /* Off first, then on. If we switched pin by pin in a loop, going from
       PC6 back to PC0 would turn PC0 on a few microseconds before PC6 is
       off, and the HIL test "never two LEDs at once" would catch it. */
    if (off) HAL_GPIO_WritePin(GPIOC, off, GPIO_PIN_RESET);
    if (on)  HAL_GPIO_WritePin(GPIOC, on,  GPIO_PIN_SET);
}

void board_toggle_heartbeat(void)
{
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);   /* LD2, the green onboard LED */
}

bool board_button_raw(void)
{
    /* B1 has a pull-up on the board: released = 1, pressed = 0. */
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET;
}
