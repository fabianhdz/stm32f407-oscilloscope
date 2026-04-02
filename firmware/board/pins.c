#include "board.h"
void board_pins_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /* Deselect the SPI1 accelerometer fitted on STM32F4 Discovery boards. */
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
    GPIO_InitTypeDef pin = {.Pin=GPIO_PIN_3,.Mode=GPIO_MODE_OUTPUT_PP,
        .Pull=GPIO_NOPULL,.Speed=GPIO_SPEED_FREQ_LOW};
    HAL_GPIO_Init(GPIOE, &pin);
    HAL_GPIO_WritePin(TFT_CONTROL_PORT, TFT_CS_PIN | TFT_RESET_PIN, GPIO_PIN_SET);
    pin.Pin = TFT_CS_PIN | TFT_DC_PIN | TFT_RESET_PIN;
    HAL_GPIO_Init(TFT_CONTROL_PORT, &pin);
    pin.Pin = BUTTON_PINS;
    pin.Mode = GPIO_MODE_INPUT;
    pin.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BUTTON_PORT, &pin);
    pin.Pin = ADC_PIN;
    pin.Mode = GPIO_MODE_ANALOG;
    pin.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ADC_PORT, &pin);
    pin.Pin = TFT_SCK_PIN | TFT_MOSI_PIN;
    pin.Mode = GPIO_MODE_AF_PP;
    pin.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    pin.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(TFT_SPI_PORT, &pin);
}
