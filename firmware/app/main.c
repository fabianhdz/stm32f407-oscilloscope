#include "board.h"
#include "drivers/uart_log.h"
void _init(void) {}
void _fini(void) {}
int main(void)
{
    board_init();
    board_pins_init();
    uart_log_init();
    uart_log("oscilloscope boot: STM32F407 HSI 160MHz, hardware unverified\r\n");
    for (;;) __WFI();
}
