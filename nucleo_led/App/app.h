/*
 * app.h - the application: glues led_seq + button + UART commands
 * to the board functions. main.c only calls these two.
 */
#ifndef APP_H
#define APP_H

void app_init(void);
void app_loop_once(void);   /* call forever from while (1) */

#endif /* APP_H */
