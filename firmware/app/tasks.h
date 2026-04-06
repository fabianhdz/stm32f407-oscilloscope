#ifndef TASKS_H
#define TASKS_H
#include "FreeRTOS.h"
#include "task.h"
extern TaskHandle_t acquisition_handle,processing_handle,input_handle,display_handle;
void app_start(void);
#endif
