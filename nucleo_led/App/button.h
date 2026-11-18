/*
 * button.h - debounce a push button and report each press once.
 *
 * A mechanical button "bounces": for a few ms after you press it the
 * signal flickers 0/1/0/1. We only believe a new level after it has
 * stayed the same for BUTTON_DEBOUNCE_MS. Pure logic, no hardware.
 */
#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>

#define BUTTON_DEBOUNCE_MS 30u

typedef struct {
    bool     stable;      /* level we believe (true = pressed)      */
    bool     last_raw;    /* level read last time                   */
    uint32_t changed_ms;  /* when last_raw last changed             */
} button_t;

void button_init(button_t *b, bool raw_pressed, uint32_t now_ms);

/* Feed the raw pin level (true = pressed). Returns true exactly once
   per press, at the moment the press becomes "stable". */
bool button_update(button_t *b, bool raw_pressed, uint32_t now_ms);

#endif /* BUTTON_H */
