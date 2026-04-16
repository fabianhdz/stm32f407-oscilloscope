#include "tasks.h"
#include "board.h"
#include "drivers/uart_log.h"
#include "drivers/sampler.h"
#include "drivers/tft_bus.h"
#include "acquisition/capture.h"
#include "app/mailbox.h"
#include "display/controls.h"
#include "display/renderer.h"
#include "processing/spectrum.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>
TaskHandle_t acquisition_handle,processing_handle,input_handle,display_handle;
static StaticTask_t tcbs[4];
static StackType_t acquisition_stack[1024],processing_stack[1024],input_stack[384],display_stack[1536];
static StaticQueue_t settings_queue_storage;
static uint8_t settings_queue_bytes[sizeof(scope_settings_t)];
static QueueHandle_t settings_queue;
static scope_settings_t settings;
static control_field_t selected_field;
static scope_calibration_t calibration;
static mailbox_t capture_box,result_box;
static scope_capture_t captures[MAILBOX_SLOTS],completed_capture;
static scope_result_t results[MAILBOX_SLOTS];
static capture_engine_t capture;
static spectrum_workspace_t spectrum __attribute__((section(".ccm"),aligned(8)));
static renderer_t renderer;
typedef struct {
    uint32_t captures,processed,displayed,acquisition_cycles_max,dsp_cycles_max,draw_cycles_max;
} runtime_stats_t;
volatile runtime_stats_t scope_runtime;
static scope_settings_t settings_snapshot(void)
{
    taskENTER_CRITICAL(); scope_settings_t copy=settings; taskEXIT_CRITICAL(); return copy;
}
static void capture_publish(void)
{
    taskENTER_CRITICAL(); int id=mailbox_write_begin(&capture_box); taskEXIT_CRITICAL();
    if(id<0) return;
    captures[id]=completed_capture;
    taskENTER_CRITICAL(); bool ok=mailbox_publish(&capture_box,(unsigned)id); taskEXIT_CRITICAL();
    configASSERT(ok);
    xTaskNotifyGive(processing_handle);
}
static void acquisition_task(void *arg)
{
    (void)arg;
    sampler_init();
    scope_settings_t local=scope_default_settings();
    local.running=false;
    for(;;) {
        scope_settings_t command;
        if(xQueueReceive(settings_queue,&command,0)==pdTRUE) {
            sampler_stop();
            local=command;
            float rate=local.running ? sampler_start(scope_requested_rate(&local)):0;
            capture_reset(&capture,&local,rate*calibration.clock_scale);
        } else if(local.running && sampler_failed()) {
            float rate=sampler_start(scope_requested_rate(&local));
            capture_reset(&capture,&local,rate*calibration.clock_scale);
        }
        if(local.running) {
            int id; uint32_t sequence;
            const uint16_t *samples;
            while((samples=sampler_take(&id,&sequence))!=NULL) {
                uint32_t begin=DWT->CYCCNT;
                bool ready=capture_feed(&capture,samples,sequence,&completed_capture);
                sampler_release(id);
                if(ready) {
                    ++scope_runtime.captures; capture_publish();
                    if(capture.stopped) {
                        sampler_stop(); local.running=false;
                        taskENTER_CRITICAL();
                        if(settings.generation==local.generation) settings.running=false;
                        taskEXIT_CRITICAL();
                    }
                }
                uint32_t cycles=DWT->CYCCNT-begin;
                if(cycles>scope_runtime.acquisition_cycles_max) scope_runtime.acquisition_cycles_max=cycles;
                if(!local.running || sampler_failed()) break;
            }
        }
        ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(10));
    }
}
static void processing_task(void *arg)
{
    (void)arg;
    configASSERT(spectrum_init(&spectrum));
    for(;;) {
        ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        taskENTER_CRITICAL(); int input=mailbox_read_begin(&capture_box); taskEXIT_CRITICAL();
        if(input<0) continue;
        scope_settings_t current=settings_snapshot();
        taskENTER_CRITICAL(); int output=mailbox_write_begin(&result_box); taskEXIT_CRITICAL();
        if(output>=0) {
            scope_result_t *r=&results[output];
            r->capture=captures[input];
            taskENTER_CRITICAL(); bool released=mailbox_read_end(&capture_box,(unsigned)input); taskEXIT_CRITICAL();
            configASSERT(released);
            uint32_t begin=DWT->CYCCNT;
            spectrum_process(&spectrum,r,&calibration);
            uint32_t cycles=DWT->CYCCNT-begin;
            if(cycles>scope_runtime.dsp_cycles_max) scope_runtime.dsp_cycles_max=cycles;
            ++scope_runtime.processed;
            /* Renderer filters generations again if settings change during the FFT. */
            r->capture.valid = r->capture.valid && r->capture.generation==current.generation;
            taskENTER_CRITICAL(); bool ok=mailbox_publish(&result_box,(unsigned)output); taskEXIT_CRITICAL();
            configASSERT(ok);
        } else {
            taskENTER_CRITICAL(); bool released=mailbox_read_end(&capture_box,(unsigned)input); taskEXIT_CRITICAL();
            configASSERT(released);
        }
        vTaskDelay(pdMS_TO_TICKS(40)); /* Bound FFT work; intermediate completed frames may be superseded. */
    }
}
static void input_task(void *arg)
{
    (void)arg;
    buttons_t buttons={0};
    TickType_t next=xTaskGetTickCount();
    for(;;) {
        uint8_t raw=(uint8_t)((~BUTTON_PORT->IDR>>BUTTON_FIRST_PIN)&15u);
        uint8_t events=buttons_poll(&buttons,raw);
        if(events) {
            taskENTER_CRITICAL();
            unsigned changes=controls_apply(&settings,&selected_field,events);
            scope_settings_t command=settings;
            taskEXIT_CRITICAL();
            if(changes&CONTROL_ACQUISITION) {
                configASSERT(xQueueOverwrite(settings_queue,&command)==pdPASS);
                xTaskNotifyGive(acquisition_handle);
            }
        }
        xTaskDelayUntil(&next,pdMS_TO_TICKS(5));
    }
}
static bool tile_sink(unsigned x,unsigned y,const uint16_t *pixels,void *ctx)
{ return ili9341_tile(ctx,x,y,16,16,pixels); }
static void diagnostics(void)
{
    char line[512];
    sampler_stats_t a=sampler_stats();
    snprintf(line,sizeof line,
        "STAT ms=%lu blocks=%lu lost=%lu ovr=%lu dmaerr=%lu late=%lu restarts=%lu nominal_rate=%lu "
        "capture=%lu fft=%lu display=%lu acq_us=%lu fft_us=%lu draw_us=%lu "
        "stack=%lu,%lu,%lu,%lu lcd_err=%lu log_drop=%lu\r\n",
        (unsigned long)HAL_GetTick(),(unsigned long)a.blocks,(unsigned long)a.dropped,
        (unsigned long)a.adc_overruns,(unsigned long)a.dma_errors,(unsigned long)a.late_irqs,
        (unsigned long)a.restarts,(unsigned long)a.nominal_rate,(unsigned long)scope_runtime.captures,
        (unsigned long)scope_runtime.processed,(unsigned long)scope_runtime.displayed,
        (unsigned long)(scope_runtime.acquisition_cycles_max/(BOARD_CPU_HZ/1000000u)),
        (unsigned long)(scope_runtime.dsp_cycles_max/(BOARD_CPU_HZ/1000000u)),
        (unsigned long)(scope_runtime.draw_cycles_max/(BOARD_CPU_HZ/1000000u)),
        (unsigned long)uxTaskGetStackHighWaterMark(acquisition_handle),
        (unsigned long)uxTaskGetStackHighWaterMark(processing_handle),
        (unsigned long)uxTaskGetStackHighWaterMark(input_handle),
        (unsigned long)uxTaskGetStackHighWaterMark(display_handle),
        (unsigned long)tft_bus_errors(),(unsigned long)uart_log_dropped());
    uart_log(line);
}
static void display_task(void *arg)
{
    (void)arg;
    ili9341_t lcd=tft_bus_init();
    bool lcd_ready=ili9341_init(&lcd);
    uint32_t last_report=HAL_GetTick(),last_retry=last_report;
    int held=-1;
    for(;;) {
        scope_settings_t current=settings_snapshot();
        taskENTER_CRITICAL();
        int newest=mailbox_read_begin(&result_box);
        control_field_t field=selected_field;
        taskEXIT_CRITICAL();
        if(newest>=0) {
            bool accept=results[newest].capture.valid &&
                results[newest].capture.generation==current.generation;
            int release=accept?held:newest;
            if(release>=0) {
                taskENTER_CRITICAL(); bool ok=mailbox_read_end(&result_box,(unsigned)release); taskEXIT_CRITICAL();
                configASSERT(ok);
            }
            if(accept) { held=newest; ++scope_runtime.displayed; }
        }
        const scope_result_t *shown=held>=0 ? &results[held]:NULL;
        if(shown && current.running && shown->capture.generation!=current.generation) shown=NULL;
        uint32_t begin=DWT->CYCCNT;
        if(lcd_ready) lcd_ready=renderer_draw(&renderer,shown,&current,field,&calibration,tile_sink,&lcd);
        else if(HAL_GetTick()-last_retry>=1000) {
            last_retry=HAL_GetTick();
            lcd=tft_bus_init();
            lcd_ready=ili9341_init(&lcd); renderer.initialized=false;
        }
        uint32_t cycles=DWT->CYCCNT-begin;
        if(cycles>scope_runtime.draw_cycles_max) scope_runtime.draw_cycles_max=cycles;
        if(HAL_GetTick()-last_report>=1000) { last_report=HAL_GetTick(); diagnostics(); }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
void app_start(void)
{
    settings=scope_default_settings(); calibration=scope_default_calibration();
    mailbox_init(&capture_box); mailbox_init(&result_box);
    settings_queue=xQueueCreateStatic(1,sizeof(scope_settings_t),settings_queue_bytes,&settings_queue_storage);
    configASSERT(settings_queue);
    configASSERT(xQueueOverwrite(settings_queue,&settings)==pdPASS);
    acquisition_handle=xTaskCreateStatic(acquisition_task,"acquisition",1024,NULL,5,acquisition_stack,&tcbs[0]);
    processing_handle=xTaskCreateStatic(processing_task,"processing",1024,NULL,4,processing_stack,&tcbs[1]);
    input_handle=xTaskCreateStatic(input_task,"input",384,NULL,3,input_stack,&tcbs[2]);
    display_handle=xTaskCreateStatic(display_task,"display",1536,NULL,2,display_stack,&tcbs[3]);
    configASSERT(acquisition_handle && processing_handle && input_handle && display_handle);
    vTaskStartScheduler();
    board_fatal("scheduler returned");
}
