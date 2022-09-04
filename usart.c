#include "util.h"
#include "usart.h"
#include "scheduler.h"

#define OSCILLATOR_FREQUENCY (8000000)

ring_buffer_t usart1_read_buffer = RING_BUFFER_DEFAULT;

void usart_enable(USART_TypeDef* usart)
{
  usart->CR1 |= (
    USART_CR1_RE |
    USART_CR1_TE |
    USART_CR1_UE | 
    USART_CR1_RXNEIE |
    USART_CR1_TXEIE
  );
}

void usart_set_baud_rate(USART_TypeDef* usart, uint32_t baud)
{
  uint16_t uartdiv = OSCILLATOR_FREQUENCY / baud;
  usart->BRR = (((uartdiv / 16) << USART_BRR_DIV_Mantissa_Pos) |
                ((uartdiv % 16) << USART_BRR_DIV_Fraction_Pos));
}

void usart_putc(USART_TypeDef* usart, const uint8_t c)
{
  while (!(usart->SR & USART_SR_TXE)) {
    scheduler_yield();
  }
  usart->DR = c;
}

void usart_getc(USART_TypeDef* usart, uint8_t* c)
{
  while (RING_BUFFER_EMPTY(&usart1_read_buffer)) {
    scheduler_yield();
  }

  *c = RING_BUFFER_PEEK(&usart1_read_buffer);
  RING_BUFFER_ADVANCE(&usart1_read_buffer, tail);
}

void usart_write(USART_TypeDef* usart, const char* buffer)
{
  while (*buffer) {
    usart_putc(usart, *(buffer++));
  }
}

void USART1_IRQHandler(void) {
  if (USART1->SR & USART_SR_RXNE) {
    char c = USART1->DR;
    if (RING_BUFFER_FULL(&usart1_read_buffer)) {
      RING_BUFFER_ADVANCE(&usart1_read_buffer, tail);
    }
    RING_BUFFER_PUT(&usart1_read_buffer, c);
    RING_BUFFER_ADVANCE(&usart1_read_buffer, head);
  }
}
