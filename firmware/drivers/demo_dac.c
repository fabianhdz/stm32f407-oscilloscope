#include "drivers/demo_dac.h"
#include "board.h"
#include <math.h>
#if SCOPE_DEMO_DAC
static uint16_t wave[64] __attribute__((section(".dma"),aligned(32)));
#endif
void demo_dac_init(void)
{
#if SCOPE_DEMO_DAC
    /* 64k samples/s / 64 entries = nominal 1kHz, roughly 0.65..2.65V.
     * Shared HSI clock means loopback cannot validate absolute frequency accuracy. */
    for(unsigned i=0;i<64;++i)
        wave[i]=(uint16_t)lroundf(2048.0f+1240.0f*sinf(6.283185307179586f*i/64.0f));
    __HAL_RCC_DAC_CLK_ENABLE(); __HAL_RCC_TIM6_CLK_ENABLE(); __HAL_RCC_DMA1_CLK_ENABLE();
    GPIO_InitTypeDef pin={.Pin=DEMO_DAC_PIN,.Mode=GPIO_MODE_ANALOG,.Pull=GPIO_NOPULL};
    HAL_GPIO_Init(DEMO_DAC_PORT,&pin);
    TIM6->CR1=0; TIM6->CR2=0; TIM6->PSC=0; TIM6->ARR=1249; TIM6->EGR=TIM_EGR_UG;
    TIM6->SR=0; TIM6->CR2=TIM_CR2_MMS_1;
    DMA1_Stream5->CR=DMA_SxCR_CHSEL|DMA_SxCR_DIR_0|DMA_SxCR_MINC|
        DMA_SxCR_MSIZE_0|DMA_SxCR_PSIZE_0|DMA_SxCR_CIRC;
    DMA1_Stream5->PAR=(uint32_t)&DAC->DHR12R1;
    DMA1_Stream5->M0AR=(uint32_t)wave; DMA1_Stream5->NDTR=64;
    DMA1_Stream5->FCR=0; DMA1_Stream5->CR |= DMA_SxCR_EN;
    DAC->DHR12R1=wave[63];
    DAC->CR=DAC_CR_EN1|DAC_CR_TEN1|DAC_CR_DMAEN1; /* TSEL1=000: TIM6 TRGO. */
    uint32_t start=DWT->CYCCNT;
    while((uint32_t)(DWT->CYCCNT-start)<BOARD_CPU_HZ/100000u) {}
    TIM6->CR1=TIM_CR1_CEN;
#endif
}
