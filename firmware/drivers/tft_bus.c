#include "drivers/tft_bus.h"
#include "board.h"
#include "app/tasks.h"
static TaskHandle_t waiter;
static volatile bool dma_error;
static uint32_t errors;
static bool wait_idle(void)
{
    uint32_t start=HAL_GetTick();
    while(!(SPI1->SR&SPI_SR_TXE) || (SPI1->SR&SPI_SR_BSY))
        if(HAL_GetTick()-start>20) return false;
    return true;
}
static void select_lcd(bool active,bool data)
{
    HAL_GPIO_WritePin(TFT_CONTROL_PORT,TFT_DC_PIN,data?GPIO_PIN_SET:GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TFT_CONTROL_PORT,TFT_CS_PIN,active?GPIO_PIN_RESET:GPIO_PIN_SET);
}
static bool bytes(bool data,const uint8_t *buffer,size_t n,void *ctx)
{
    (void)ctx;
    if(!wait_idle()) { ++errors; return false; }
    SPI1->CR1 &= ~(SPI_CR1_SPE|SPI_CR1_DFF);
    SPI1->CR1 |= SPI_CR1_SPE;
    select_lcd(true,data);
    for(size_t i=0;i<n;++i) {
        uint32_t start=HAL_GetTick();
        while(!(SPI1->SR&SPI_SR_TXE))
            if(HAL_GetTick()-start>20) { select_lcd(false,data); ++errors; return false; }
        *(__IO uint8_t *)&SPI1->DR=buffer[i];
    }
    bool ok=wait_idle(); select_lcd(false,data);
    if(!ok) ++errors;
    return ok;
}
static bool pixels(const uint16_t *buffer,size_t n,void *ctx)
{
    (void)ctx;
    if(!n || n>65535 || !wait_idle()) { ++errors; return false; }
    waiter=xTaskGetCurrentTaskHandle();
    (void)ulTaskNotifyTake(pdTRUE,0);
    dma_error=false;
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 |= SPI_CR1_DFF;
    SPI1->CR1 |= SPI_CR1_SPE;
    DMA2_Stream3->CR=DMA_SxCR_CHSEL_0|DMA_SxCR_CHSEL_1|DMA_SxCR_DIR_0|
        DMA_SxCR_MINC|DMA_SxCR_MSIZE_0|DMA_SxCR_PSIZE_0|DMA_SxCR_TCIE|DMA_SxCR_TEIE|DMA_SxCR_DMEIE;
    DMA2_Stream3->PAR=(uint32_t)&SPI1->DR;
    DMA2_Stream3->M0AR=(uint32_t)buffer;
    DMA2_Stream3->NDTR=(uint32_t)n;
    DMA2_Stream3->FCR=DMA_SxFCR_FEIE;
    DMA2->LIFCR=DMA_LIFCR_CFEIF3|DMA_LIFCR_CDMEIF3|DMA_LIFCR_CTEIF3|DMA_LIFCR_CHTIF3|DMA_LIFCR_CTCIF3;
    select_lcd(true,true);
    SPI1->CR2 |= SPI_CR2_TXDMAEN;
    DMA2_Stream3->CR |= DMA_SxCR_EN;
    bool ok=ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(50))!=0 && !dma_error;
    SPI1->CR2 &= ~SPI_CR2_TXDMAEN;
    DMA2_Stream3->CR &= ~DMA_SxCR_EN;
    uint32_t start=HAL_GetTick();
    while(DMA2_Stream3->CR&DMA_SxCR_EN)
        if(HAL_GetTick()-start>20) board_fatal("TFT DMA stop");
    /* DMA TC precedes the final SPI shift. Keep CS and buffer stable until BSY clears. */
    if(!wait_idle()) ok=false;
    select_lcd(false,true);
    if(!ok) ++errors;
    return ok;
}
static void reset(bool high,void *ctx)
{ (void)ctx; HAL_GPIO_WritePin(TFT_CONTROL_PORT,TFT_RESET_PIN,high?GPIO_PIN_SET:GPIO_PIN_RESET); }
static void delay(uint32_t ms,void *ctx) { (void)ctx; vTaskDelay(pdMS_TO_TICKS(ms)); }
ili9341_t tft_bus_init(void)
{
    __HAL_RCC_SPI1_CLK_ENABLE(); __HAL_RCC_DMA2_CLK_ENABLE();
    /* APB2=80MHz / 8 = 10MHz. One-line output avoids an unused receive overrun. */
    SPI1->CR1=SPI_CR1_MSTR|SPI_CR1_SSM|SPI_CR1_SSI|SPI_CR1_BR_1|
              SPI_CR1_BIDIMODE|SPI_CR1_BIDIOE|SPI_CR1_SPE;
    SPI1->CR2=0;
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn,7,0); HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
    return (ili9341_t){bytes,pixels,reset,delay,NULL};
}
void DMA2_Stream3_IRQHandler(void)
{
    uint32_t flags=DMA2->LISR;
    DMA2->LIFCR=DMA_LIFCR_CFEIF3|DMA_LIFCR_CDMEIF3|DMA_LIFCR_CTEIF3|DMA_LIFCR_CHTIF3|DMA_LIFCR_CTCIF3;
    if(flags&(DMA_LISR_TEIF3|DMA_LISR_DMEIF3|DMA_LISR_FEIF3)) dma_error=true;
    SPI1->CR2 &= ~SPI_CR2_TXDMAEN;
    BaseType_t wake=pdFALSE;
    if(waiter) vTaskNotifyGiveFromISR(waiter,&wake);
    portYIELD_FROM_ISR(wake);
}
uint32_t tft_bus_errors(void) { return errors; }
