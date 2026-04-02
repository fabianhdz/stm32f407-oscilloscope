#ifndef BOARD_H
#define BOARD_H
#include "stm32f4xx_hal.h"
#define BOARD_CPU_HZ 160000000u
#define BOARD_APB1_TIMER_HZ 80000000u
#define BOARD_APB2_HZ 80000000u
/* Change pins together with the wiring table and peripheral alternate functions. */
#define ADC_PORT GPIOA
#define ADC_PIN GPIO_PIN_1
#define ADC_CHANNEL ADC_CHANNEL_1
#define TFT_SPI_PORT GPIOA
#define TFT_SCK_PIN GPIO_PIN_5
#define TFT_MOSI_PIN GPIO_PIN_7
#define TFT_CONTROL_PORT GPIOB
#define TFT_CS_PIN GPIO_PIN_6
#define TFT_DC_PIN GPIO_PIN_7
#define TFT_RESET_PIN GPIO_PIN_8
#define BUTTON_PORT GPIOB
#define BUTTON_FIRST_PIN 12u
#define BUTTON_PINS (GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)
#define UART_PORT GPIOA
#define UART_TX_PIN GPIO_PIN_2
#define UART_RX_PIN GPIO_PIN_3
#define DEMO_DAC_PORT GPIOA
#define DEMO_DAC_PIN GPIO_PIN_4
void board_init(void);
void board_pins_init(void);
void board_fatal(const char *reason);
#endif
