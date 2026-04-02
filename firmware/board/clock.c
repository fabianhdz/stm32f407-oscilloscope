#include "board.h"
void board_init(void)
{
    HAL_Init();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    RCC_OscInitTypeDef osc = {0};
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    osc.PLL.PLLM = 16;
    osc.PLL.PLLN = 320;
    osc.PLL.PLLP = RCC_PLLP_DIV2;
    osc.PLL.PLLQ = 7; /* USB is intentionally not clocked from this profile. */
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) board_fatal("PLL");
    RCC_ClkInitTypeDef clk = {0};
    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV4;
    clk.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_5) != HAL_OK) board_fatal("clock");
    SCB->VTOR = FLASH_BASE;
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}
/* HAL timeouts must work before and after the scheduler starts. TIM5 owns the
 * HAL millisecond timebase; FreeRTOS exclusively owns SysTick. */
HAL_StatusTypeDef HAL_InitTick(uint32_t priority)
{
    __HAL_RCC_TIM5_CLK_ENABLE();
    uint32_t hz = HAL_RCC_GetPCLK1Freq();
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0) hz *= 2;
    TIM5->CR1 = 0;
    TIM5->PSC = hz / 1000000u - 1u;
    TIM5->ARR = 999;
    TIM5->EGR = TIM_EGR_UG;
    TIM5->SR = 0;
    TIM5->DIER = TIM_DIER_UIE;
    HAL_NVIC_SetPriority(TIM5_IRQn, priority, 0);
    HAL_NVIC_EnableIRQ(TIM5_IRQn);
    TIM5->CR1 = TIM_CR1_CEN;
    return HAL_OK;
}
void HAL_SuspendTick(void) { TIM5->DIER &= ~TIM_DIER_UIE; }
void HAL_ResumeTick(void) { TIM5->DIER |= TIM_DIER_UIE; }
void TIM5_IRQHandler(void)
{
    if (TIM5->SR & TIM_SR_UIF) { TIM5->SR = 0; HAL_IncTick(); }
}
