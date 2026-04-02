#ifndef UART_LOG_H
#define UART_LOG_H
#include <stdint.h>
void uart_log_init(void);
/* Single task producer after boot; nonblocking, drops whole messages if full. */
void uart_log(const char *text);
uint32_t uart_log_dropped(void);
#endif
