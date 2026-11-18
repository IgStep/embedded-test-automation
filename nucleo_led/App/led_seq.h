/*
 * led_seq.h - the "brain" of the running light.
 *
 * This module only does logic: which LED is on, which direction, how fast.
 * It never touches hardware (no HAL, no GPIO). That is what makes it
 * testable on a PC: you give it a time, it tells you what should happen.
 */
#ifndef LED_SEQ_H          /* "include guard": stops the file being      */
#define LED_SEQ_H          /*  pasted twice if two files include it      */

#include <stdint.h>        /* uint8_t, uint32_t: integers with exact size */
#include <stdbool.h>       /* bool, true, false                           */

#define LED_COUNT          7u      /* PC0..PC6                     */
#define LED_PERIOD_DEFAULT 1000u   /* ms each LED stays on         */
#define LED_PERIOD_MIN     125u    /* fastest allowed              */
#define LED_PERIOD_MAX     2000u   /* slowest allowed              */

/* All the state of the sequence lives in one struct.
   The caller owns it, so tests can create as many as they like. */
typedef struct {
    uint8_t  index;         /* which LED is on now, 0..LED_COUNT-1     */
    int8_t   dir;           /* +1 = forward (PC0->PC6), -1 = backward  */
    uint32_t period_ms;     /* how long each LED stays on              */
    uint32_t last_step_ms;  /* time of the last step                   */
    bool     paused;
} led_seq_t;

void    led_seq_init(led_seq_t *s, uint32_t now_ms);

/* Call often with the current time. Returns true when the sequence moved
   to the next LED (so the caller knows to update the pins). */
bool    led_seq_update(led_seq_t *s, uint32_t now_ms);

/* Bit i set = LED i on. Example: index 2 -> 0b0000100 = 0x04. */
uint8_t led_seq_mask(const led_seq_t *s);

void    led_seq_reverse(led_seq_t *s);
void    led_seq_faster(led_seq_t *s);   /* halves period, stops at MIN  */
void    led_seq_slower(led_seq_t *s);   /* doubles period, stops at MAX */
void    led_seq_toggle_pause(led_seq_t *s, uint32_t now_ms);

#endif /* LED_SEQ_H */
