#include "board.h"
void _init(void) {}
void _fini(void) {}
int main(void)
{
    board_init();
    board_pins_init();
    for (;;) __WFI();
}
