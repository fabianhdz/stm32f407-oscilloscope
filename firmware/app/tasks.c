#include "tasks.h"
#include "board.h"
#include "drivers/uart_log.h"
TaskHandle_t acquisition_handle,processing_handle,input_handle,display_handle;
static StaticTask_t tcbs[4];
static StackType_t acquisition_stack[1024],processing_stack[1024],input_stack[384],display_stack[1024];
static void acquisition_task(void *arg)
{ (void)arg; for(;;) ulTaskNotifyTake(pdTRUE,portMAX_DELAY); }
static void processing_task(void *arg)
{ (void)arg; for(;;) ulTaskNotifyTake(pdTRUE,portMAX_DELAY); }
static void input_task(void *arg)
{ (void)arg; for(;;) vTaskDelay(pdMS_TO_TICKS(5)); }
static void display_task(void *arg)
{
    (void)arg;
    uart_log("FreeRTOS static tasks started\r\n");
    for(;;) vTaskDelay(pdMS_TO_TICKS(1000));
}
void app_start(void)
{
    acquisition_handle=xTaskCreateStatic(acquisition_task,"acquisition",1024,NULL,5,acquisition_stack,&tcbs[0]);
    processing_handle=xTaskCreateStatic(processing_task,"processing",1024,NULL,4,processing_stack,&tcbs[1]);
    input_handle=xTaskCreateStatic(input_task,"input",384,NULL,3,input_stack,&tcbs[2]);
    display_handle=xTaskCreateStatic(display_task,"display",1024,NULL,2,display_stack,&tcbs[3]);
    configASSERT(acquisition_handle && processing_handle && input_handle && display_handle);
    vTaskStartScheduler();
    board_fatal("scheduler returned");
}
