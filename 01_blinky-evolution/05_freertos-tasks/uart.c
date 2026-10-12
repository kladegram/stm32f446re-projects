/* USART2 setup and polling transmit, adapted from ../04_uart-debug/main.c. */
#include "stm32f4xx.h"
#include "uart.h"

#define PCLK1_HZ 16000000U
#define DEBUG_BAUD 115200U
#define DEBUG_TX_AF 7U

void USART2_Init(void)
{
  /* USART2 and GPIOA are separate peripherals, so both need clocks. */
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART2EN);
  (void)READ_BIT(RCC->APB1ENR, RCC_APB1ENR_USART2EN);
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
  (void)READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);

  /* Connect PA2 to USART2_TX using alternate function 7. */
  MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFSEL2,
             DEBUG_TX_AF << GPIO_AFRL_AFSEL2_Pos);
  MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER2, GPIO_MODER_MODER2_1);

  /* 115200 baud, 8 data bits, no parity, 1 stop bit (reset defaults). */
  WRITE_REG(USART2->BRR, (PCLK1_HZ + DEBUG_BAUD / 2U) / DEBUG_BAUD);
  SET_BIT(USART2->CR1, USART_CR1_TE | USART_CR1_UE);
}

void USART_write(int ch)
{
  while (READ_BIT(USART2->SR, USART_SR_TXE) == 0U)
  {
    /* Wait for space in the transmit data register. */
  }
  WRITE_REG(USART2->DR, (uint8_t)ch);
}

void USART_write_string(const char *text)
{
  while (*text != '\0')
  {
    USART_write(*text++);
  }
}

