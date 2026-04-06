#include "tasks.h"
#include "board.h"
static StaticTask_t idle_tcb;
static StackType_t idle_stack[configMINIMAL_STACK_SIZE];
void vApplicationGetIdleTaskMemory(StaticTask_t **tcb,StackType_t **stack,uint32_t *depth)
{ *tcb=&idle_tcb; *stack=idle_stack; *depth=configMINIMAL_STACK_SIZE; }
void vApplicationStackOverflowHook(TaskHandle_t task,char *name)
{ (void)task; (void)name; board_fatal("task stack overflow"); }
