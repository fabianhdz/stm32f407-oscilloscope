#include "stm32f4xx_hal.h"
void _init(void) {}
void _fini(void) {}
int main(void)
{
    for (;;) __WFI();
}
