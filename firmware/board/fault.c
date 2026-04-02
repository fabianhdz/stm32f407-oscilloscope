#include "board.h"
volatile const char *scope_fault_reason;
void board_fatal(const char *reason)
{
    scope_fault_reason = reason;
    __disable_irq();
    for (;;) __WFI(); /* Inspect scope_fault_reason with the debugger. */
}
void HardFault_Handler(void) { board_fatal("HardFault"); }
void MemManage_Handler(void) { board_fatal("MemManage"); }
void BusFault_Handler(void) { board_fatal("BusFault"); }
void UsageFault_Handler(void) { board_fatal("UsageFault"); }
