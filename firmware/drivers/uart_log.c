#include "uart_log.h"
#include "board.h"
#include <string.h>
#define TX_SIZE 2048u
static char tx[TX_SIZE];
static volatile uint32_t head, tail;
static uint32_t dropped;
void uart_log_init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();
    GPIO_InitTypeDef pin = {.Pin=UART_TX_PIN | UART_RX_PIN,.Mode=GPIO_MODE_AF_PP,
        .Pull=GPIO_PULLUP,.Speed=GPIO_SPEED_FREQ_HIGH,.Alternate=GPIO_AF7_USART2};
    HAL_GPIO_Init(UART_PORT, &pin);
    USART2->CR1 = 0;
    USART2->BRR = (HAL_RCC_GetPCLK1Freq() + 57600u) / 115200u;
    USART2->CR2 = 0;
    USART2->CR3 = 0;
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE;
    HAL_NVIC_SetPriority(USART2_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}
void uart_log(const char *text)
{
    size_t n = strlen(text);
    if (n >= TX_SIZE || n > TX_SIZE - 1u - ((head - tail) & (TX_SIZE - 1u))) {
        ++dropped;
        return;
    }
    uint32_t next = head;
    for (size_t i=0; i<n; ++i) tx[(next++) & (TX_SIZE-1u)] = text[i];
    __DMB();
    head = next;
    USART2->CR1 |= USART_CR1_TXEIE;
}
uint32_t uart_log_dropped(void) { return dropped; }
void USART2_IRQHandler(void)
{
    if ((USART2->SR & USART_SR_TXE) && (USART2->CR1 & USART_CR1_TXEIE)) {
        if (tail != head) USART2->DR = (uint8_t)tx[(tail++) & (TX_SIZE-1u)];
        else USART2->CR1 &= ~USART_CR1_TXEIE;
    }
}
