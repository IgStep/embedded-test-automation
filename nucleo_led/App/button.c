#include "button.h"

void button_init(button_t *b, bool raw_pressed, uint32_t now_ms)
{
    b->stable     = raw_pressed;
    b->last_raw   = raw_pressed;
    b->changed_ms = now_ms;
}

bool button_update(button_t *b, bool raw_pressed, uint32_t now_ms)
{
    if (raw_pressed != b->last_raw) {      /* level moved: restart timer */
        b->last_raw   = raw_pressed;
        b->changed_ms = now_ms;
        return false;
    }

    if (raw_pressed != b->stable &&
        (uint32_t)(now_ms - b->changed_ms) >= BUTTON_DEBOUNCE_MS) {
        b->stable = raw_pressed;           /* level held long enough     */
        return b->stable;                  /* event only on press, not on release */
    }
    return false;
}
