/*
 * host_sim.c - board.h implemented for a Linux/macOS PC ("host SIL").
 *
 * Lets the system test suite (tests/system) run without any hardware,
 * so you can develop and debug the Python tests on your PC first.
 *   stdout : the UART output (BOOT, STEP ...)
 *   stderr : pin changes, one line each:  PIN PC3 1
 *   stdin  : UART input. Two special bytes simulate the button:
 *            0x01 = press B1, 0x02 = release B1
 * Real time is used, so 1000 ms really takes 1 s.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "board.h"
#include "app.h"

static struct timespec t0;
static bool button_down = false;
static uint8_t leds = 0;
static bool ld2 = false;

uint32_t board_millis(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint32_t)((now.tv_sec - t0.tv_sec) * 1000 +
                      (now.tv_nsec - t0.tv_nsec) / 1000000);
}

void board_set_leds(uint8_t mask)
{
    for (int i = 0; i < 7; i++) {          /* report off-changes first */
        if ((leds >> i & 1u) && !(mask >> i & 1u)) fprintf(stderr, "PIN PC%d 0\n", i);
    }
    for (int i = 0; i < 7; i++) {
        if (!(leds >> i & 1u) && (mask >> i & 1u)) fprintf(stderr, "PIN PC%d 1\n", i);
    }
    leds = mask;
}

void board_toggle_heartbeat(void)
{
    ld2 = !ld2;
    fprintf(stderr, "PIN PA5 %d\n", ld2 ? 1 : 0);
}

bool board_button_raw(void) { return button_down; }

int board_uart_getc(void)
{
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;   /* non-blocking */
    if (c == 0x01) { button_down = true;  return -1; }
    if (c == 0x02) { button_down = false; return -1; }
    return c;
}

void board_uart_puts(const char *s)
{
    fputs(s, stdout);
    fflush(stdout);
}

int main(void)
{
    clock_gettime(CLOCK_MONOTONIC, &t0);
    setvbuf(stderr, NULL, _IOLBF, 0);
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);

    app_init();
    for (;;) {
        app_loop_once();
        struct timespec nap = { 0, 200000 };   /* 0.2 ms, don't burn a CPU core */
        nanosleep(&nap, NULL);
    }
}