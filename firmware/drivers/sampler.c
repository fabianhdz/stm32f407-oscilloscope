#include "drivers/sampler.h"
#include "acquisition/buffer_pool.h"
#include "board.h"
#include "app/tasks.h"
static uint16_t buffers[SCOPE_DMA_BUFFERS][SCOPE_BLOCK_SAMPLES]
    __attribute__((section(".dma"),aligned(32)));
static buffer_pool_t pool;
static volatile bool failed;
static volatile sampler_stats_t stats;
static uint32_t sequence,last_irq,expected_cycles;
static void notify_acquisition(void)
{
    BaseType_t wake=pdFALSE;
    vTaskNotifyGiveFromISR(acquisition_handle,&wake);
    portYIELD_FROM_ISR(wake);
}
void sampler_init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE(); __HAL_RCC_TIM2_CLK_ENABLE(); __HAL_RCC_DMA2_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn,6,0); HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    HAL_NVIC_SetPriority(ADC_IRQn,6,0); HAL_NVIC_EnableIRQ(ADC_IRQn);
}
void sampler_stop(void)
{
    TIM2->CR1=0;
    stats.nominal_rate=0;
    ADC1->CR1=0;
    ADC1->CR2=0;
    DMA2_Stream0->CR &= ~DMA_SxCR_EN;
    uint32_t start=DWT->CYCCNT;
    while(DMA2_Stream0->CR&DMA_SxCR_EN)
        if((uint32_t)(DWT->CYCCNT-start)>BOARD_CPU_HZ/100u) board_fatal("ADC DMA stop");
    DMA2->LIFCR=DMA_LIFCR_CFEIF0|DMA_LIFCR_CDMEIF0|DMA_LIFCR_CTEIF0|DMA_LIFCR_CHTIF0|DMA_LIFCR_CTCIF0;
    NVIC_ClearPendingIRQ(DMA2_Stream0_IRQn); NVIC_ClearPendingIRQ(ADC_IRQn);
}
float sampler_start(uint32_t requested_hz)
{
    configASSERT(requested_hz>=10000 && requested_hz<=1000000);
    sampler_stop();
    taskENTER_CRITICAL();
    buffer_pool_reset(&pool);
    failed=false; sequence=0; last_irq=0; ++stats.restarts;
    taskEXIT_CRITICAL();
    uint32_t divisor=(BOARD_APB1_TIMER_HZ+requested_hz/2u)/requested_hz;
    float actual=(float)BOARD_APB1_TIMER_HZ/(float)divisor;
    stats.nominal_rate=(uint32_t)actual;
    expected_cycles=(uint32_t)(((uint64_t)BOARD_CPU_HZ*SCOPE_BLOCK_SAMPLES*divisor)/BOARD_APB1_TIMER_HZ);
    /* Generate update while ADC external triggering is disabled. */
    TIM2->CR2=0; TIM2->PSC=0; TIM2->ARR=divisor-1; TIM2->CNT=0;
    TIM2->EGR=TIM_EGR_UG; TIM2->SR=0; TIM2->CR2=TIM_CR2_MMS_1;
    ADC->CCR=ADC_CCR_ADCPRE_0; /* PCLK2 / 4 = 20MHz. */
    ADC1->SMPR1=0; ADC1->SMPR2=0; /* 3 acquisition + 12 conversion cycles = 0.75us. */
    ADC1->SQR1=0; ADC1->SQR2=0; ADC1->SQR3=ADC_CHANNEL;
    ADC1->SR=0; ADC1->CR1=ADC_CR1_OVRIE;
    DMA2_Stream0->CR=DMA_SxCR_MINC|DMA_SxCR_MSIZE_0|DMA_SxCR_PSIZE_0|
        DMA_SxCR_PL_1|DMA_SxCR_DBM|DMA_SxCR_CIRC|DMA_SxCR_TCIE|DMA_SxCR_TEIE|DMA_SxCR_DMEIE;
    DMA2_Stream0->PAR=(uint32_t)&ADC1->DR;
    DMA2_Stream0->M0AR=(uint32_t)buffers[0];
    DMA2_Stream0->M1AR=(uint32_t)buffers[1];
    DMA2_Stream0->NDTR=SCOPE_BLOCK_SAMPLES;
    DMA2_Stream0->FCR=DMA_SxFCR_FEIE;
    DMA2_Stream0->CR |= DMA_SxCR_EN;
    ADC1->CR2=ADC_CR2_DMA|ADC_CR2_DDS|ADC_EXTERNALTRIGCONV_T2_TRGO|ADC_CR2_EXTEN_0|ADC_CR2_ADON;
    /* ADC stabilization before the first timer edge; no sample loop. */
    uint32_t start=DWT->CYCCNT;
    while((uint32_t)(DWT->CYCCNT-start)<BOARD_CPU_HZ/100000u) {}
    TIM2->CR1=TIM_CR1_CEN;
    return actual;
}
static bool replace_buffer(unsigned slot,unsigned replacement,void *ctx)
{
    (void)ctx;
    unsigned active=(DMA2_Stream0->CR&DMA_SxCR_CT) ? 1u:0u;
    if(active==slot) return false;
    if(slot==0) DMA2_Stream0->M0AR=(uint32_t)buffers[replacement];
    else DMA2_Stream0->M1AR=(uint32_t)buffers[replacement];
    __DMB();
    return true;
}
void DMA2_Stream0_IRQHandler(void)
{
    uint32_t flags=DMA2->LISR;
    DMA2->LIFCR=DMA_LIFCR_CFEIF0|DMA_LIFCR_CDMEIF0|DMA_LIFCR_CTEIF0|DMA_LIFCR_CHTIF0|DMA_LIFCR_CTCIF0;
    if(flags&(DMA_LISR_TEIF0|DMA_LISR_DMEIF0|DMA_LISR_FEIF0)) {
        ++stats.dma_errors; failed=true; TIM2->CR1=0;
    }
    if((flags&DMA_LISR_TCIF0) && !failed) {
        uint32_t now=DWT->CYCCNT;
        if(last_irq && (uint32_t)(now-last_irq)>expected_cycles+expected_cycles/2u) {
            ++stats.late_irqs; failed=true; TIM2->CR1=0;
        }
        last_irq=now;
        if(!failed) {
            unsigned completed=(DMA2_Stream0->CR&DMA_SxCR_CT) ? 0u:1u;
            if(!buffer_pool_complete(&pool,completed,sequence++,replace_buffer,NULL)) ++stats.dropped;
            ++stats.blocks;
        }
    }
    notify_acquisition();
}
void ADC_IRQHandler(void)
{
    if(ADC1->SR&ADC_SR_OVR) {
        TIM2->CR1=0; ADC1->CR1 &= ~ADC_CR1_OVRIE;
        ADC1->SR &= ~ADC_SR_OVR;
        ++stats.adc_overruns; failed=true;
        notify_acquisition();
    }
}
bool sampler_failed(void) { return failed; }
const uint16_t *sampler_take(int *id,uint32_t *seq)
{
    taskENTER_CRITICAL(); *id=buffer_pool_take(&pool,seq); taskEXIT_CRITICAL();
    return *id<0 ? NULL:buffers[*id];
}
void sampler_release(int id)
{
    taskENTER_CRITICAL(); bool ok=buffer_pool_release(&pool,(unsigned)id); taskEXIT_CRITICAL();
    configASSERT(ok);
}
sampler_stats_t sampler_stats(void)
{
    taskENTER_CRITICAL(); sampler_stats_t copy=stats; taskEXIT_CRITICAL();
    return copy;
}
