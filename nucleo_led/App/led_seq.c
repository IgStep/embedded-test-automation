#include "led_seq.h"

void led_seq_init(led_seq_t *s, uint32_t now_ms)
{
    /* s is a pointer: "s->index" means "the index field of the struct
       that s points to". Same as (*s).index. */
    s->index        = 0;
    s->dir          = +1;
    s->period_ms    = LED_PERIOD_DEFAULT;
    s->last_step_ms = now_ms;
    s->paused       = false;
}

bool led_seq_update(led_seq_t *s, uint32_t now_ms)
{
    if (s->paused) {
        return false;
    }

    /* Unsigned subtraction is safe even when the millisecond counter
       wraps from 0xFFFFFFFF back to 0 (after ~49.7 days):
       e.g. 5 - 0xFFFFFFFB = 10 in uint32_t arithmetic. A test checks it. */
    if ((uint32_t)(now_ms - s->last_step_ms) < s->period_ms) {
        return false;                       /* not time yet */
    }

    /* += period (instead of = now) keeps the rhythm exact: if we are
       called 3 ms late, the next step is still on the 1000 ms grid. */
    s->last_step_ms += s->period_ms;

    if (s->dir > 0) {
        s->index = (uint8_t)((s->index + 1u) % LED_COUNT);   /* 6 -> 0 */
    } else {
        s->index = (s->index == 0u) ? (uint8_t)(LED_COUNT - 1u)  /* 0 -> 6 */
                                    : (uint8_t)(s->index - 1u);
    }
    return true;
}

uint8_t led_seq_mask(const led_seq_t *s)
{
    /* 1u << 3 = 0b1000. "const" promises we only read *s. */
    return (uint8_t)(1u << s->index);
}

void led_seq_reverse(led_seq_t *s)
{
    s->dir = (int8_t)(-s->dir);
}

void led_seq_faster(led_seq_t *s)
{
    s->period_ms /= 2u;
    if (s->period_ms < LED_PERIOD_MIN) {
        s->period_ms = LED_PERIOD_MIN;
    }
}

void led_seq_slower(led_seq_t *s)
{
    s->period_ms *= 2u;
    if (s->period_ms > LED_PERIOD_MAX) {
        s->period_ms = LED_PERIOD_MAX;
    }
}

void led_seq_toggle_pause(led_seq_t *s, uint32_t now_ms)
{
    s->paused = !s->paused;
    if (!s->paused) {
        /* On resume, restart the timer, so the LED we paused on
           gets a full period instead of jumping immediately. */
        s->last_step_ms = now_ms;
    }
}
